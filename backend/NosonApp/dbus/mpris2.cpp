/*
 *      Copyright (C) 2019 Jean-Luc Barriere
 *
 *  This file is part of Noson-App
 *
 *  Noson is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Noson is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Noson.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mpris2.h"

#include <QGuiApplication>
#include <QDBusConnection>
#include <QDebug>

#include "mpris2_root.h"
#include "mpris2_player.h"
#include "player.h"
#include "tools.h"

#define MPRIS_OBJECT_PATH     "/org/mpris/MediaPlayer2"
#define DBUS_MEDIAPLAYER_SVC  "org.mpris.MediaPlayer2"
#define DBUS_FREEDESKTOP_SVC  "org.freedesktop.DBus.Properties"

using namespace nosonapp;

Mpris2::Mpris2(Player* app, QObject* parent)
: QObject(parent)
, m_player(app)
, m_registered(false)
, m_identity()
, m_serviceName()
, m_servicePath()
, m_metadata()
{
  new Mpris2Root(this);
  new Mpris2Player(this);

  if (m_player)
  {
    QObject::connect(m_player, SIGNAL(connectedChanged(int)), SLOT(connectionStateChanged(int)));
    QObject::connect(m_player, SIGNAL(playbackStateChanged(int)), SLOT(playbackStateChanged(int)));
    QObject::connect(m_player, SIGNAL(renderingGroupChanged(int)), SLOT(volumeChanged(int)));
    QObject::connect(m_player, SIGNAL(playModeChanged(int)), SLOT(playModeChanged(int)));
    QObject::connect(m_player, SIGNAL(sourceChanged(int)), SLOT(currentTrackChanged(int)));
    initDBusService(m_player->pid());
  }
}

Mpris2::~Mpris2()
{
  if (m_registered)
    QDBusConnection::sessionBus().unregisterService(m_serviceName);
}

void Mpris2::connectionStateChanged(int pid)
{
  initDBusService(pid);
}

void Mpris2::playbackStateChanged(int pid)
{
  emitPlayerNotification("CanPlay", CanPlay());
  emitPlayerNotification("CanPause", CanPause());
  emitPlayerNotification("PlaybackStatus", PlaybackStatus());
  if (m_player->playbackState() == "PLAYING")
    emitPlayerNotification("CanSeek", CanSeek());
}

void Mpris2::volumeChanged(int pid)
{
  emitPlayerNotification("Volume", Volume());
}

void Mpris2::playModeChanged(int pid)
{
  emitPlayerNotification("Shuffle", Shuffle());
  emitPlayerNotification("LoopStatus", LoopStatus());
  emitPlayerNotification("CanGoNext", CanGoNext());
  emitPlayerNotification("CanGoPrevious", CanGoPrevious());
}

void Mpris2::initDBusService(int pid)
{
  if (m_registered)
    QDBusConnection::sessionBus().unregisterService(m_serviceName);
  m_registered = false;

  if (!m_player->connected())
    return;

  // Try to make a friendly name for the player according to the Dbus specs.
  // A valid name must only contain the ASCII characters "[A-Z][a-z][0-9]_" and
  // must not begin with a digit.
  QString zoneId;
  QString zone_ = normalizedString(m_player->zoneShortName().split('+').front());
  foreach (QChar c, zone_)
  {
    switch (c.category())
    {
    case QChar::Letter_Lowercase:
    case QChar::Letter_Uppercase:
    case QChar::Number_DecimalDigit:
      zoneId.append(c);
      break;
    default:
      zoneId.append('_');
    }
  }


  m_identity = QString("%1.%2").arg(QGuiApplication::applicationDisplayName(), zoneId);

  m_servicePath = QString("/%1/%2")
          .arg(QString(QGuiApplication::applicationName()).replace('.', '/'), zoneId);

  m_serviceName = QString(DBUS_MEDIAPLAYER_SVC ".%1.%2")
          .arg(QGuiApplication::applicationDisplayName(), zoneId);

  if (!QDBusConnection::sessionBus().registerService(m_serviceName))
  {
    qWarning() << "Failed to register" << m_serviceName << "on the session bus";
    return;
  }
  m_registered = true;
  QDBusConnection::sessionBus().registerObject(MPRIS_OBJECT_PATH, this);

  m_metadata = QVariantMap();
  currentTrackChanged(pid);
  playbackStateChanged(pid);
  playModeChanged(pid);
  emitPlayerNotification("Volume", Volume());

  qDebug() << "Succeeded to register" << m_serviceName << "on the session bus";
}

void Mpris2::emitPlayerNotification(const QString& name, const QVariant& val)
{
  emitNotification(name, val, DBUS_MEDIAPLAYER_SVC ".Player");
}

void Mpris2::emitNotification(const QString& name, const QVariant& val, const QString& mprisEntity)
{
  QDBusMessage msg = QDBusMessage::createSignal(MPRIS_OBJECT_PATH, DBUS_FREEDESKTOP_SVC, "PropertiesChanged");
  QVariantMap map;
  map.insert(name, val);
  QVariantList args = QVariantList() << mprisEntity << map << QStringList();
  msg.setArguments(args);
  QDBusConnection::sessionBus().send(msg);
}

QString Mpris2::Identity() const
{
  return m_identity;
}

QString Mpris2::desktopEntryAbsolutePath() const
{
  QString appId = DesktopEntry();
  QStringList xdg_data_dirs = QString(getenv("XDG_DATA_DIRS")).split(":");
  xdg_data_dirs.append("/usr/local/share/");
  xdg_data_dirs.append("/usr/share/");

  for (const QString& directory : xdg_data_dirs)
  {
    QString path = QString("%1/applications/%2.desktop")
            .arg(directory, appId);
    if (QFile::exists(path))
      return path;
  }
  return QString();
}

QString Mpris2::DesktopEntry() const
{
  return QGuiApplication::applicationName().toLower();
}

QStringList Mpris2::SupportedUriSchemes() const
{
  static QStringList res = QStringList()
          << "file"
          << "http";
  return res;
}

QStringList Mpris2::SupportedMimeTypes() const
{
  static QStringList res = QStringList()
          << "audio/aac"
          << "audio/mp3"
          << "audio/flac"
          << "audio/ogg"
          << "application/ogg"
          << "audio/x-mp3"
          << "audio/x-flac"
          << "application/x-ogg";
  return res;
}

void Mpris2::Raise()
{
}

void Mpris2::Quit()
{
}

QString Mpris2::PlaybackStatus() const
{
  QString state = m_player->playbackState();
  if (state == "PLAYING")
    return "Playing";
  if (state == "PAUSED_PLAYBACK")
    return "Paused";
  return "Stopped";
}

QString Mpris2::LoopStatus() const
{
  QString mode = m_player->playMode();
  if (mode == "SHUFFLE")
    return "Playlist";
  if (mode == "REPEAT_ALL")
    return "Playlist";
  if (mode == "REPEAT_ONE")
    return "Track";
  return "None";
}

void Mpris2::SetLoopStatus(const QString& value)
{
  QString mode = m_player->playMode();
  if ((value == "None" && (mode == "REPEAT_ALL" || mode == "SHUFFLE" || mode == "REPEAT_ONE")) ||
          (value == "Playlist" && (mode == "NORMAL" || mode == "SHUFFLE_NOREPEAT")))
    m_player->toggleRepeat();
}

double Mpris2::Rate() const
{
  return 1.0;
}

void Mpris2::SetRate(double rate)
{
  if (rate == 0)
    m_player->pause();
}

bool Mpris2::Shuffle() const
{
  QString mode = m_player->playMode();
  return (mode == "SHUFFLE" || mode == "SHUFFLE_NOREPEAT");
}

void Mpris2::SetShuffle(bool enable)
{
  QString mode = m_player->playMode();
  if ((mode == "SHUFFLE" || mode == "SHUFFLE_NOREPEAT") != enable)
    m_player->toggleShuffle();
}

QVariantMap Mpris2::Metadata() const
{
  return m_metadata;
}

QString Mpris2::makeTrackId(int index) const
{
  return QString("%1/track/%2").arg(m_servicePath).arg(QString::number(index));
}

void Mpris2::currentTrackChanged(int pid)
{
  emitPlayerNotification("CanPlay", CanPlay());
  emitPlayerNotification("CanPause", CanPause());
  emitPlayerNotification("CanGoNext", CanGoNext());
  emitPlayerNotification("CanGoPrevious", CanGoPrevious());
  emitPlayerNotification("CanSeek", CanSeek());

  m_metadata = QVariantMap();
  addMetadata("mpris:trackid", makeTrackId(m_player->currentIndex()), &m_metadata);
  addMetadata("mpris:length", (qint64)(1000000L * m_player->currentTrackDuration()), &m_metadata);
  addMetadata("mpris:artUrl", m_player->currentMetaArt(), &m_metadata);
  addMetadata("xesam:title", m_player->currentMetaTitle(), &m_metadata);
  addMetadata("xesam:album", m_player->currentMetaAlbum(), &m_metadata);
  addMetadataAsList("xesam:artist", m_player->currentMetaArtist(), &m_metadata);

  emitPlayerNotification("Metadata", m_metadata);
}

double Mpris2::Volume() const
{
  return (double) (m_player->volumeMaster()) / 100.0;
}

void Mpris2::SetVolume(double value)
{
  m_player->setVolumeGroup(value * 100.0);
}

qlonglong Mpris2::Position() const
{
  return 1000000L * m_player->currentTrackPosition();
}

double Mpris2::MaximumRate() const
{
  return 1.0;
}

double Mpris2::MinimumRate() const
{
  return 1.0;
}

bool Mpris2::CanGoNext() const
{
  return m_player->canGoNext();
}

bool Mpris2::CanGoPrevious() const
{
  return m_player->canGoPrevious();
}

bool Mpris2::CanPlay() const
{
  return true;
}

bool Mpris2::CanPause() const
{
  return true;
}

bool Mpris2::CanSeek() const
{
  return m_player->canSeek();
}

bool Mpris2::CanControl() const
{
  return true;
}

void Mpris2::Next()
{
  if (CanGoNext())
    m_player->next();
}

void Mpris2::Previous()
{
  if (CanGoPrevious())
    m_player->previous();
}

void Mpris2::Pause()
{
  if (CanPause() && m_player->playbackState() == "PLAYING")
    m_player->pause();
}

void Mpris2::PlayPause()
{
  if (CanPause())
  {
    QString state = m_player->playbackState();
    if (state == "PLAYING")
      m_player->pause();
    else if (state == "STOPPED" || state == "PAUSED_PLAYBACK")
      m_player->play();
  }
}

void Mpris2::Stop()
{
  m_player->stop();
}

void Mpris2::Play()
{
  if (CanPlay())
  {
    m_player->play();
  }
}

void Mpris2::Seek(qlonglong offset)
{
  if (CanSeek())
    m_player->seekTime(m_player->currentTrackPosition() + offset / 1000000L);
}

void Mpris2::SetPosition(const QDBusObjectPath& trackId, qlonglong offset)
{
  if (CanSeek() && trackId.path() == makeTrackId(m_player->currentIndex()) && offset >= 0)
    m_player->seekTime(offset / 1000000L);
}

void Mpris2::OpenUri(const QString& uri)
{
  Q_UNUSED(uri);
}

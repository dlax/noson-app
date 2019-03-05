/*
 *      Copyright (C) 2019 Jean-Luc Barriere
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef ID3PARSER_H
#define ID3PARSER_H

#include "mediaparser.h"

namespace mediascanner
{

class ID3Parser : public MediaParser
{
public:
  const char * commonName() override { return "ID3"; }
  bool match(const QFileInfo& fileInfo) override;
  bool parse(MediaFile * file, MediaInfo * info, bool debug) override;
};

}

#endif /* ID3PARSER_H */


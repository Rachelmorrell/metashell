#ifndef METASHELL_PRAGMA_HELP_HPP
#define METASHELL_PRAGMA_HELP_HPP

// Metashell - Interactive C++ template metaprogramming shell
// Copyright (C) 2014, Abel Sinkovics (abel@sinkovics.hu)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <metashell/iface/displayer.hpp>
#include <metashell/iface/pragma_handler.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace metashell
{
  namespace core
  {
    class pragma_help : public iface::pragma_handler
    {
    public:
      explicit pragma_help(
          const std::map<std::vector<std::string>,
                         std::unique_ptr<iface::pragma_handler>>&
              pragma_handlers_);

      virtual std::string arguments() const override;
      virtual std::string description() const override;

      virtual void run(const data::command::iterator& name_begin_,
                       const data::command::iterator& name_end_,
                       const data::command::iterator& args_begin_,
                       const data::command::iterator& args_end_,
                       iface::displayer& displayer_) const override;

    private:
      const std::map<std::vector<std::string>,
                     std::unique_ptr<iface::pragma_handler>>& _pragma_handlers;
    };
  }
}

#endif

// Metashell - Interactive C++ template metaprogramming shell
// Copyright (C) 2016, Abel Sinkovics (abel@sinkovics.hu)
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

#include <metashell/engine/vc/header_discoverer.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/optional.hpp>

#include <just/environment.hpp>
#include <just/lines.hpp>

#include <algorithm>
#include <regex>

namespace metashell
{
  namespace engine
  {
    namespace vc
    {
      namespace
      {
        boost::optional<boost::filesystem::path>
        include_path_addition(const data::command_line_argument& arg_)
        {
          if ((starts_with(arg_, data::command_line_argument("\"/I")) ||
               starts_with(arg_, data::command_line_argument("/I\""))) &&
              ends_with(arg_, data::command_line_argument("\"")))
          {
            return boost::filesystem::path(
                substr(arg_, 3, size(arg_) - 4).value());
          }
          else if (starts_with(arg_, data::command_line_argument("/I")))
          {
            return boost::filesystem::path(substr(arg_, 2).value());
          }

          return boost::none;
        }

        data::includes
        determine_includes(const data::command_line_argument_list& cl_args_)
        {
          const std::string include = just::environment::get("INCLUDE");

          data::includes result{{}, {"."}};

          boost::algorithm::split(
              result.sys, include, boost::algorithm::is_any_of(";"));

          result.sys.erase(
              std::remove_if(
                  result.sys.begin(), result.sys.end(),
                  [](const boost::filesystem::path& p_) { return p_.empty(); }),
              result.sys.end());

          for (const data::command_line_argument& arg : cl_args_)
          {
            if (const boost::optional<boost::filesystem::path> path =
                    include_path_addition(arg))
            {
              result.sys.push_back(*path);
            }
          }

          result.quote.insert(
              result.quote.end(), result.sys.begin(), result.sys.end());

          return result;
        }
      }

      header_discoverer::header_discoverer(binary binary_)
        : _binary(binary_), _includes(determine_includes(binary_.base_args()))
      {
      }

      std::vector<boost::filesystem::path>
      header_discoverer::include_path(data::include_type type_)
      {
        return get(type_, _includes);
      }

      std::set<boost::filesystem::path>
      header_discoverer::files_included_by(const data::cpp_code& exp_)
      {
        const data::process_output output =
            run(_binary, {"/c", "/showIncludes"}, exp_);

        const std::regex included_header("^[^:]+: [^:]+: *");

        std::set<boost::filesystem::path> result;

        for (const std::string line : just::lines::view(output.standard_output))
        {
          std::smatch what;
          if (regex_search(line, what, included_header))
          {
            result.insert(what.suffix().str());
          }
        }

        return result;
      }
    }
  }
}

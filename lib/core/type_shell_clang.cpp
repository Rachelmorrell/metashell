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

#include <metashell/type_shell_clang.hpp>

#include <metashell/exception.hpp>
#include <metashell/metashell.hpp>

#include <boost/algorithm/string/trim.hpp>

namespace metashell
{
  type_shell_clang::type_shell_clang(
      const boost::filesystem::path& internal_dir_,
      const boost::filesystem::path& env_filename_,
      clang_binary clang_binary_,
      logger* logger_)
    : _clang_binary(clang_binary_),
      _env_path(internal_dir_ / env_filename_),
      _logger(logger_)
  {
  }

  data::result type_shell_clang::eval(
      const iface::environment& env_,
      const boost::optional<std::string>& tmp_exp_,
      const boost::optional<boost::filesystem::path>& templight_dump_path_,
      bool use_precompiled_headers_)
  {
    std::vector<std::string> clang_args{"-Xclang", "-ast-dump"};
    if (use_precompiled_headers_)
    {
      clang_args.push_back("-include");
      clang_args.push_back(_env_path.string());
    }
    if (templight_dump_path_)
    {
      clang_args.push_back("-Xtemplight");
      clang_args.push_back("-profiler");
      clang_args.push_back("-Xtemplight");
      clang_args.push_back("-safe-mode");

      // templight can't be forced to generate output file with
      // -Xtemplight -output=<file> for some reason
      // A workaround is to specify a standard output location with -o
      // then append ".trace.pbf" to the specified file (on the calling side)
      clang_args.push_back("-o");
      clang_args.push_back(templight_dump_path_->string());
    }

    const data::process_output output =
        run_clang(_clang_binary, clang_args,
                  tmp_exp_ ?
                      env_.get_appended("::metashell::impl::wrap< " +
                                        *tmp_exp_ + " > __metashell_v;\n") :
                      env_.get());

    const bool success = output.exit_code == data::exit_code_t(0);

    return data::result{success,
                        success && tmp_exp_ ?
                            get_type_from_ast_string(output.standard_output) :
                            "",
                        success ? "" : output.standard_error, ""};
  }

  data::result type_shell_clang::validate_code(const std::string& src_,
                                               const data::config& config_,
                                               const iface::environment& env_,
                                               bool use_precompiled_headers_)
  {
    METASHELL_LOG(_logger, "Validating code " + src_);

    try
    {
      const std::string src = env_.get_appended(src_);
      std::vector<std::string> clang_args{"-fsyntax-only"};
      if (use_precompiled_headers_)
      {
        clang_args.push_back("-include");
        clang_args.push_back(_env_path.string());
      }

      const data::process_output output =
          run_clang(_clang_binary, clang_args, src);

      const bool accept = output.exit_code == data::exit_code_t(0) &&
                          output.standard_error.empty();

      return data::result{accept, "", output.standard_error,
                          accept && config_.verbose ? src : ""};
    }
    catch (const std::exception& e)
    {
      return data::result(false, "", e.what(), "");
    }
  }

  void type_shell_clang::generate_precompiled_header(
      const boost::filesystem::path& fn_)
  {
    using boost::algorithm::trim_copy;

    const std::string filename = fn_.string();

    METASHELL_LOG(_logger, "Generating percompiled header for " + filename);

    std::vector<std::string> args{
        "-iquote", ".", "-w", "-o", filename + ".pch", filename};

    const data::process_output o = _clang_binary.run(args, "");
    const std::string err = o.standard_output + o.standard_error;
    if (!err.empty()
        // clang displays this even when "-w" is used. This can be ignored
        &&
        trim_copy(err) !=
            "warning: precompiled header used __DATE__ or __TIME__.")
    {
      throw exception("Error precompiling header " + filename + ": " + err);
    }
  }
}

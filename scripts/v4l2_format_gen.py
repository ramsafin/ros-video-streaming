#!/usr/bin/env python3
# encoding: utf-8

import os
import re
import json
from argparse import ArgumentParser
from typing import Final, List, Dict, Any
from dataclasses import dataclass, asdict


V4L2_HEADER_FILEPATH: Final[os.PathLike]= "/usr/include/linux/videodev2.h"

V4L2_PIX_FMT_PATTERN: Final[str] = \
    r'#define\s+(V4L2_PIX_FMT_\w+)\s+v4l2_fourcc\(\s*\'(.)\',\s*\'(.)\',\s*\'(.)\',\s*\'(.)\'\s*\)'


@dataclass
class PixelFormat:
  name: str
  value: str
  fourcc: str


def parse_v4l2_header(filepath: os.PathLike) -> List[PixelFormat]:
    pattern = re.compile(V4L2_PIX_FMT_PATTERN)
    formats: List[PixelFormat] = []

    with open(filepath, "r") as header_file:
        for line in header_file:
            match = pattern.search(line)
            if match:
                value, c1, c2, c3, c4 = match.groups()
                fourcc = "".join([c1, c2, c3, c4])
                name = f"V4L2_{fourcc}"
                formats.append(PixelFormat(name.strip(), value.strip(), fourcc.strip()))

    return formats


def save_as_json(formats: List[PixelFormat], out_filepath: os.PathLike) -> None:
    with open(out_filepath, 'w') as out:
        json.dump([asdict(fmt) for fmt in formats], out, indent=2)


def load_from_json(filepath: os.PathLike) -> List[PixelFormat]:
    with open(filepath) as f:
        return [PixelFormat(**entry) for entry in json.load(f)]


def generate_header(formats: List[PixelFormat], out_filepath: os.PathLike) -> None:
    with open(out_filepath, "w") as out:
        out.write("// AUTO-GENERATED V4L2 PIXEL FORMAT HEADER\n")
        out.write("#ifndef LIRS_V4L2_FORMATS_HPP\n")
        out.write("#define LIRS_V4L2_FORMATS_HPP\n\n")

        out.write("#include <linux/videodev2.h>\n\n")
        out.write("#include <cstdint>\n")
        out.write("#include <optional>\n")
        out.write("#include <string_view>\n")
        out.write("#include <type_traits>\n\n")

        out.write("namespace lirs::formats {\n\n")

        # Enum declaration
        out.write("enum class PixelFormat : uint32_t {\n")
        for fmt in formats:
            out.write(f"  {fmt.name} = {fmt.value},\n")
        out.write("};\n\n")

        # format2str function
        out.write("inline constexpr std::string_view format2str(PixelFormat fmt) {\n")
        out.write("  switch (fmt) {\n")
        for fmt in formats:
            out.write(f'    case PixelFormat::{fmt.name}:\n      return "{fmt.value}";\n')
        out.write("    default:\n      return \"UNKNOWN\";\n")
        out.write("  }\n")
        out.write("}\n\n")

        # fourcc2format function
        out.write("inline constexpr std::optional<PixelFormat> fourcc2format(uint32_t fourcc) {\n")
        out.write("  switch (fourcc) {\n")
        for fmt in formats:
            out.write(f"    case {fmt.value}:\n      return PixelFormat::{fmt.name};\n")
        out.write("    default:\n      return std::nullopt;\n")
        out.write("  }\n")
        out.write("}\n\n")

        #  underlying type
        out.write("inline constexpr uint32_t format2fourcc(PixelFormat fmt) {\n")
        out.write("  return static_cast<uint32_t>(fmt);\n")
        out.write("}\n\n")

        out.write("}  // namespace lirs::formats\n")
        out.write("#endif  // LIRS_V4L2_FORMATS_HPP\n")


def parse_args() -> Dict[str, Any]:
    parser = ArgumentParser(description="V4L2 Pixel Format Tool")
    subparsers = parser.add_subparsers(dest="command", required=True)

    # parse
    parse_cmd = subparsers.add_parser("parse", help="Parse V4L2 header to JSON")
    parse_cmd.add_argument("--header", help="V4L2 header file (default: %(default)s)", default=V4L2_HEADER_FILEPATH)
    parse_cmd.add_argument("--out", help="Output JSON file", required=True)

    # generate
    gen_cmd = subparsers.add_parser("generate", help="Generate C++ header from JSON")
    gen_cmd.add_argument("--in", dest="in_file", help="Input JSON file", required=True)
    gen_cmd.add_argument("--out", help="Output C++ header file", required=True)

    # all
    all_cmd = subparsers.add_parser("all", help="Parse header and generate C++ header")
    all_cmd.add_argument("--header", help="V4L2 header file (default: %(default)s)", default=V4L2_HEADER_FILEPATH)
    all_cmd.add_argument("--out", help="Output C++ header file", required=True)

    return parser.parse_args()


def main() -> None:
    args: Dict[str, Any] = parse_args()

    if args.command == "parse":
        formats = parse_v4l2_header(args.header)
        save_as_json(formats, args.out)

    elif args.command == "generate":
        formats = load_from_json(args.in_file)
        generate_header(formats, args.out)

    elif args.command == "all":
        formats = parse_v4l2_header(args.header)
        generate_header(formats, args.out)


if __name__ == "__main__":
    main()

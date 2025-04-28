{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    esp-dev.url = "github:mirrexagon/nixpkgs-esp-dev";
    esp-dev.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs =
    {
      self,
      nixpkgs,
      esp-dev,
    }:
    {
      devShells.x86_64-linux.default =
        let
          pkgs = import nixpkgs { system = "x86_64-linux"; };
        in
        pkgs.mkShell {
          name = "esp-idf-esp32s3-shell";

          buildInputs = [
            (esp-dev.packages.x86_64-linux.esp-idf-esp32s3.override {
              toolsToInclude = [
                "xtensa-esp-elf"
                "esp32ulp-elf"
                "openocd-esp32"
                "xtensa-esp-elf-gdb"
                "esp-clang"
              ];
            })
          ];

          # IDF_TOOLCHAIN = "clang";
        };
    };
}

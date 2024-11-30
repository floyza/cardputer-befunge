{
  description = "A very basic flake";

  inputs = {
    esp-dev.url = "github:mirrexagon/nixpkgs-esp-dev";
  };

  outputs =
    { self, esp-dev }:
    {
      devShells.x86_64-linux.default =
        let
          pkgs = import esp-dev.inputs.nixpkgs { system = "x86_64-linux"; };
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

          IDF_TOOLCHAIN = "clang";
        };
    };
}

{
  description = "VECTx development environment and package";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      # Flakes must be pure, so the systems come from nixpkgs rather than from
      # whatever happens to be running.
      eachSystem = nixpkgs.lib.genAttrs (
        nixpkgs.lib.systems.flakeExposed
      );
      pkgsFor = eachSystem (system:
        nixpkgs.legacyPackages.${system}
      );
    in {
      devShells = eachSystem (system: {
        default = (pkgsFor.${system}).mkShell {
          packages = with pkgsFor.${system}; [
            cmake
            gcc

            # cmake defaults to the Makefiles generator, so a build tool has to
            # be on PATH. gcc's stdenv happens to bring gnumake in, but relying
            # on that is a hidden dependency: name it instead.
            gnumake
          ];
        };
      });

      # Builds the demo binary from the checkout. The demo reads two lines of
      # input, so run it with something piped in:
      #   printf '21\nhello\n' | nix run .
      packages = eachSystem (system:
        let
          pkgs = pkgsFor.${system};
        in {
          default = pkgs.stdenv.mkDerivation {
            pname = "vectx";
            version = "0.6.2";

            src = ./.;

            nativeBuildInputs = with pkgs; [
              cmake
              makeWrapper
            ];

            cmakeFlags = [ "-DCMAKE_BUILD_TYPE=Release" ];

            doCheck = true;

            # CMake writes into ./build here and RUNTIME_OUTPUT_DIRECTORY puts
            # the binary in ./build/bin, matching what a local `cmake -B build`
            # produces. The docs are read from the source root.
            installPhase = ''
              runHook preInstall

              # cmakePhase runs the build from inside ./build, and
              # installPhase inherits that working directory, so the binary is
              # ./bin/vectx here and the docs are one level up.
              install -Dm755 bin/vectx $out/bin/vectx
              install -Dm644 ../README.md $out/share/doc/vectx/README.md
              install -Dm644 ../LICENSE $out/share/doc/vectx/LICENSE

              runHook postInstall
            '';

            meta = with pkgs.lib; {
              description = "A small dynamically typed scripting language";
              license = licenses.mit;
              platforms = platforms.unix;
            };
          };
        }
      );

      apps = eachSystem (system: {
        default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/vectx";
          meta.description = "A small dynamically typed scripting language";
        };
      });
    };
}
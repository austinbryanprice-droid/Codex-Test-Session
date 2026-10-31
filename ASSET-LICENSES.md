# Asset provenance

- **MakeHuman hm08 body**: MakeHuman Community, Data Collection AB, Joel Palmius,
  Jonas Hauquier. CC0, explicitly stated in the source OBJ header (September 2020).
  Source: https://raw.githubusercontent.com/makehumancommunity/makehuman/master/makehuman/data/3dobjs/base.obj
  The actual downloaded file and its exact notice/SHA256 are preserved in
  `art-source/makehuman-base.obj` and `art-source/MAKEHUMAN-CC0.txt`.
  This project uses only the body group; helper anatomy is excluded.
- **Original generated derivative**: heroic body deformation, SDF wolf skull,
  surface-extracted armor, swept mane, teeth, claws, ornaments, rig, six clips,
  cloth, arena and lighting are authored in the project's code. Generated art is
  supplied under CC0. Source code is supplied under MIT (LICENSE).
- **Three.js 0.180.0** and its RoomEnvironment/OrbitControls/CSM/glTF utilities: MIT.
  Its RoomEnvironment is a generated studio environment, not a downloaded HDR.
- **scikit-image**, used only to mesh the original signed-distance field during
  generation: BSD-3-Clause software; no third-party model is included from it.
  Blender is a build-time tool and is not redistributed.

No Quaternius asset or animation has been integrated. The shared rig and clips
are original; there is no claim of retargeting a third-party animation library.
No external fonts, texture packs, paid content, or reference-image pixels are
bundled. The attached user image guides silhouette/color; the design and geometry
are original. This is a stylized approximation, not an exact reproduction.

CC0 legal terms: https://creativecommons.org/publicdomain/zero/1.0/legalcode

The directional fur-detail, tangent-normal and cutout PNGs are original code-generated
CC0 art (`scripts/generate-fur.py`), not scanned or copied from the reference.
The asset-pack ZIP uses the MIT-licensed fflate utility bundled with Three.js.

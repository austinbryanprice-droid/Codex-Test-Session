"""Run with UnrealEditor-Cmd -ExecutePythonScript=<this file>, UE 5.5.4.
Creates original material assets only; never downloads third-party content.
"""
import unreal
assets = unreal.AssetToolsHelpers.get_asset_tools()
colors = {
    'Ground': (0.13, 0.18, 0.10), 'Field': (0.25, 0.23, 0.12),
    'Road': (0.055, 0.063, 0.068), 'Concrete': (0.31, 0.33, 0.32),
    'Water': (0.06, 0.14, 0.18), 'Objective': (0.46, 0.52, 0.23),
    'TeamA': (0.12, 0.29, 0.24), 'TeamB': (0.40, 0.30, 0.17),
}
for name, color in colors.items():
    path = '/Game/Generated/M_' + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        continue
    material = assets.create_asset('M_' + name, '/Game/Generated', unreal.Material, unreal.MaterialFactoryNew())
    base = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant3Vector)
    base.set_editor_property('constant', unreal.LinearColor(*color, 1.0))
    rough = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant)
    rough.set_editor_property('r', 0.28 if name == 'Water' else 0.82)
    unreal.MaterialEditingLibrary.connect_material_property(base, '', unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(rough, '', unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_asset(path):
        raise RuntimeError('Could not save ' + path)
unreal.log('Frontline original materials prepared.')

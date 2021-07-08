"""
Copyright (c) Contributors to the Open 3D Engine Project. For complete copyright and license terms please see the LICENSE at the root of this distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT
"""

import os
import sys

import azlmbr.legacy.general as general
import azlmbr.bus as bus
import azlmbr.math as math
import azlmbr.paths

sys.path.append(os.path.join(azlmbr.paths.devroot, 'AutomatedTesting', 'Gem', 'PythonTests'))
import editor_python_test_tools.hydra_editor_utils as hydra
from editor_python_test_tools.editor_test_helper import EditorTestHelper
from largeworlds.large_worlds_utils import editor_dynveg_test_helper as dynveg


class TestDynamicSliceInstanceSpawner(EditorTestHelper):
    def __init__(self):
        EditorTestHelper.__init__(self, log_prefix="DynamicSliceInstanceSpawner", args=["level"])

    def run_test(self):
        """
        Summary:
        Test aspects of the DynamicSliceInstanceSpawner through the BehaviorContext and the Property Tree.

        :return: None
        """
        # 1) Open an empty level
        self.test_success = self.create_level(
            self.args["level"],
            heightmap_resolution=1024,
            heightmap_meters_per_pixel=1,
            terrain_texture_resolution=4096,
            use_terrain=False,
        )
        general.idle_wait(1.0)
        general.set_current_view_position(512.0, 480.0, 38.0)

        # Grab the UUID that we need for creating an Dynamic Slice Instance Spawner
        dynamic_slice_spawner_uuid = azlmbr.math.Uuid_CreateString('{BBA5CC1E-B4CA-4792-89F7-93711E98FBD1}', 0)

        # Grab a path to a test dynamic slice asset
        test_slice_asset_path = os.path.join("Slices", "PinkFlower.dynamicslice")

        # 2) Test DynamicSliceInstanceSpawner BehaviorContext
        behavior_context_test_success = True
        dynamic_slice_spawner = azlmbr.vegetation.DynamicSliceInstanceSpawner()
        behavior_context_test_success = behavior_context_test_success and (dynamic_slice_spawner is not None)
        behavior_context_test_success = behavior_context_test_success and (dynamic_slice_spawner.typename == 'DynamicSliceInstanceSpawner')
        # Try to get/set the slice asset path with a valid asset
        dynamic_slice_spawner.SetSliceAssetPath(test_slice_asset_path)
        validate_path = dynamic_slice_spawner.GetSliceAssetPath()
        # We expect the path to get lowercased and normalized with a forward slash, so we compare our result
        # vs that instead of directly against test_slice_asset_path.
        behavior_context_test_success = behavior_context_test_success and hydra.compare_values('slices/pinkflower.dynamicslice', validate_path, 'GetSliceAssetPath - valid')
        # Try to get/set the slice asset path with an empty path
        dynamic_slice_spawner.SetSliceAssetPath('')
        validate_path = dynamic_slice_spawner.GetSliceAssetPath()
        behavior_context_test_success = behavior_context_test_success and hydra.compare_values('', validate_path, 'GetSliceAssetPath - empty')
        self.test_success = self.test_success and behavior_context_test_success
        self.log(f'DynamicSliceInstanceSpawner() BehaviorContext test: {behavior_context_test_success}')

        # 3) Test Descriptor BehaviorContext - setting spawnerType sets spawner too
        spawner_type_test_success = True
        descriptor = azlmbr.vegetation.Descriptor()
        spawner_type_test_success = spawner_type_test_success and hydra.get_set_property_test(descriptor, 'spawnerType', dynamic_slice_spawner_uuid)
        spawner_type_test_success = spawner_type_test_success and (descriptor.spawner.typename == 'DynamicSliceInstanceSpawner')
        self.test_success = self.test_success and spawner_type_test_success
        self.log(f'Descriptor() BehaviorContext spawnerType test: {spawner_type_test_success}')

        # 4) Test Descriptor BehaviorContext - setting spawner sets spawnerType too
        spawner_test_success = True
        descriptor = azlmbr.vegetation.Descriptor()
        descriptor.spawner = dynamic_slice_spawner
        spawner_test_success = spawner_test_success and (descriptor.spawnerType.Equal(dynamic_slice_spawner_uuid))
        spawner_test_success = spawner_test_success and (descriptor.spawner.typename == 'DynamicSliceInstanceSpawner')
        self.test_success = self.test_success and spawner_test_success
        self.log(f'Descriptor() BehaviorContext spawner test: {spawner_test_success}')

        ### Setup for Property Tree set of tests

        # Create a new entity with required vegetation area components
        spawner_entity = hydra.Entity("Veg Area")
        spawner_entity.create_entity(
            math.Vector3(512.0, 512.0, 32.0), 
            ["Vegetation Layer Spawner", "Box Shape", "Vegetation Asset List"]
            )
        if (spawner_entity.id.IsValid()):
            self.log(f"'{spawner_entity.name}' created")

        # Resize the Box Shape component
        new_box_dimensions = math.Vector3(16.0, 16.0, 16.0)
        box_dimensions_path = "Box Shape|Box Configuration|Dimensions"
        spawner_entity.get_set_test(1, box_dimensions_path, new_box_dimensions)

        # Create a surface to plant on
        dynveg.create_surface_entity("Surface Entity", math.Vector3(512.0, 512.0, 32.0), 1024.0, 1024.0, 1.0)

        # 5) Descriptor Property Tree test: spawner type can be set

        # - Validate the dynamic slice spawner type can be set correctly.
        property_tree_success = True
        property_tree_success = property_tree_success and spawner_entity.get_set_test(2, 'Configuration|Embedded Assets|[0]|Instance Spawner', dynamic_slice_spawner_uuid)

        # This should result in 400 instances, since our box is 16 m x 16 m and by default the veg system plants
        # 20 instances per 16 meters
        spawner_entity.get_set_test(0, 'Configuration|Allow Empty Assets', True)
        num_expected_instances = 20 * 20
        property_tree_success = property_tree_success and self.wait_for_condition(lambda: dynveg.validate_instance_count_in_entity_shape(spawner_entity.id, num_expected_instances), 5.0)
        self.test_success = self.test_success and property_tree_success
        self.log(f'Property Tree spawner type test: {property_tree_success}')

        # 6) Validate that the "Allow Empty Assets" setting affects the DynamicSliceInstanceSpawner
        allow_empty_assets_success = True
        # Since we have an empty slice path, we should have 0 instances once we disable 'Allow Empty Assets'
        num_expected_instances = 0
        allow_empty_assets_success = allow_empty_assets_success and spawner_entity.get_set_test(0, 'Configuration|Allow Empty Assets', False)
        self.log('Allow Empty Assets test:')
        allow_empty_assets_success = allow_empty_assets_success and self.wait_for_condition(lambda: dynveg.validate_instance_count_in_entity_shape(spawner_entity.id, num_expected_instances), 5.0)
        self.test_success = self.test_success and allow_empty_assets_success
        self.log(f'Allow Empty Assets test: {allow_empty_assets_success}')

        # 7) Validate that with 'Allow Empty Assets' set to False, a non-empty slice asset gives us the number
        # of instances we expect.
        spawns_slices_success = True
        num_expected_instances = 20 * 20
        dynamic_slice_spawner.SetSliceAssetPath(test_slice_asset_path)
        spawns_slices_success = spawns_slices_success and spawner_entity.get_set_test(0, 'Configuration|Allow Empty Assets', False)
        descriptor = hydra.get_component_property_value(spawner_entity.components[2], 'Configuration|Embedded Assets|[0]')
        descriptor.spawner = dynamic_slice_spawner
        spawns_slices_success = spawns_slices_success and spawner_entity.get_set_test(2, "Configuration|Embedded Assets|[0]", descriptor)
        self.log('Spawn dynamic slices test:')
        spawns_slices_success = spawns_slices_success and self.wait_for_condition(lambda: dynveg.validate_instance_count_in_entity_shape(spawner_entity.id, num_expected_instances), 5.0)
        self.test_success = self.test_success and spawns_slices_success
        self.log(f'Spawn dynamic slices test: {spawns_slices_success}')


test = TestDynamicSliceInstanceSpawner()
test.run()

"""
All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
its licensors.

For complete copyright and license terms please see the LICENSE at the root of this
distribution (the "License"). All use of this software is governed by the License,
or, if provided, by the license below or the license accompanying this file. Do not
remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
"""

import os, sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import azlmbr.asset as asset
import azlmbr.bus as bus
import azlmbr.components as components
import azlmbr.editor as editor
import azlmbr.legacy.general as general
import azlmbr.math as math

sys.path.append(os.path.join(azlmbr.paths.devroot, 'AutomatedTesting', 'Gem', 'PythonTests'))
import automatedtesting_shared.hydra_editor_utils as hydra
from automatedtesting_shared.editor_test_helper import EditorTestHelper
from largeworlds.large_worlds_utils import editor_dynveg_test_helper as dynveg


class test_MeshBlocker_InstancesBlockedByMesh(EditorTestHelper):
    def __init__(self):
        EditorTestHelper.__init__(self, log_prefix="MeshBlocker_InstancesBlockedByMesh", args=["level"])

    def run_test(self):
        """
        Summary:
        Level is created. An entity with a vegetation spawner and entity with vegetation blocker (Mesh) component are
        added. Finally, the instance counts are checked to verify expected numbers after blocker is applied.

        Expected Behavior:
        The vegetation planted in the Spawner area is blocked by the Mesh of the Vegetation Blocker Mesh component.

        Test Steps:
         --> Create level
         --> Create Spawner Entity
         --> Create Surface Entity to spawn vegetation instances on
         --> Create Blocker Entity with cube mesh
         --> Verify spawned vegetation instance counts

        Note:
        - Any passed and failed tests are written to the Editor.log file.
                Parsing the file or running a log_monitor are required to observe the test results.

        :return: None
        """

        # Create a new level
        self.test_success = self.create_level(
            self.args["level"],
            heightmap_resolution=1024,
            heightmap_meters_per_pixel=1,
            terrain_texture_resolution=4096,
            use_terrain=False,
        )

        general.set_current_view_position(500.49, 498.69, 46.66)
        general.set_current_view_rotation(-42.05, 0.00, -36.33)

        # Create entity with components "Vegetation Layer Spawner", "Vegetation Asset List", "Box Shape"
        entity_position = math.Vector3(512.0, 512.0, 32.0)
        asset_path = os.path.join("Slices", "PurpleFlower.dynamicslice")
        spawner_entity = dynveg.create_vegetation_area("Instance Spawner",
                                                       entity_position,
                                                       10.0, 10.0, 10.0,
                                                       asset_path)

        # Create surface entity to plant on
        dynveg.create_surface_entity("Surface Entity", entity_position, 10.0, 10.0, 1.0)

        # Create blocker entity with cube mesh
        mesh_type_id = azlmbr.globals.property.EditorMeshComponentTypeId
        blocker_entity = hydra.Entity("Blocker Entity")
        blocker_entity.create_entity(entity_position,
                                     ["Vegetation Layer Blocker (Mesh)"])
        blocker_entity.add_component_of_type(mesh_type_id)
        if blocker_entity.id.IsValid():
            print(f"'{blocker_entity.name}' created")
        cubeId = asset.AssetCatalogRequestBus(
            bus.Broadcast, "GetAssetIdByPath", os.path.join("objects", "_primitives", "_box_1x1.azmodel"), math.Uuid(),
            False)
        blocker_entity.get_set_test(1, "Controller|Configuration|Mesh Asset", cubeId)
        components.TransformBus(bus.Event, "SetLocalScale", blocker_entity.id, math.Vector3(2.0, 2.0, 2.0))

        # Disable/Re-enable Mesh component due to ATOM-14299
        general.idle_wait(1.0)
        editor.EditorComponentAPIBus(bus.Broadcast, 'DisableComponents', [blocker_entity.components[1]])
        is_enabled = editor.EditorComponentAPIBus(bus.Broadcast, 'IsComponentEnabled', blocker_entity.components[1])
        if is_enabled:
            print("Mesh component is still enabled")
        else:
            print("Mesh component was disabled")
        editor.EditorComponentAPIBus(bus.Broadcast, 'EnableComponents', [blocker_entity.components[1]])
        is_enabled = editor.EditorComponentAPIBus(bus.Broadcast, 'IsComponentEnabled', blocker_entity.components[1])
        if is_enabled:
            print("Mesh component is now enabled")
        else:
            print("Mesh component is still disabled")

        # Verify spawned instance counts are accurate after addition of Blocker Entity
        num_expected = 160  # Number of "PurpleFlower"s that plant on a 10 x 10 surface minus 2m blocker cube
        result = self.wait_for_condition(lambda: dynveg.validate_instance_count_in_entity_shape(spawner_entity.id,
                                                                                                num_expected), 2.0)
        self.test_success = self.test_success and result


test = test_MeshBlocker_InstancesBlockedByMesh()
test.run()

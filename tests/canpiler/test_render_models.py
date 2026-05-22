import contextlib
import io
import sys
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
CANPILER_DIR = REPO_ROOT / "can_library" / "canpiler"
sys.path.insert(0, str(CANPILER_DIR))

from codegen import build_node_render_context, build_signal_codec
from faultgen import augment_system_with_faults
from linker import link_all
from mapper import map_hardware
from parser import Message, Signal, create_system_context, load_bus_configs, load_custom_types, parse_all


def build_context():
    with contextlib.redirect_stdout(io.StringIO()):
        nodes = parse_all()
        bus_configs = load_bus_configs()
        custom_types = load_custom_types()
        augment_system_with_faults(nodes, bus_configs, custom_types)
        link_all(nodes)
        mappings = map_hardware(nodes, bus_configs)
    return create_system_context(nodes, mappings, bus_configs, custom_types)


class RenderModelTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.context = build_context()
        cls.nodes = {node.name: node for node in cls.context.nodes}

    def test_a_box_render_context_has_expected_entries(self):
        ctx = build_node_render_context(self.nodes["A_BOX"], self.context)

        rx = {entry.msg.name: entry for entry in ctx.rx_entries}
        tx = {entry.msg.name: entry for entry in ctx.tx_entries}
        scaling_names = {entry.msg.name for entry in ctx.scaling_messages}
        stale_names = {entry.msg.name for entry in ctx.stale_rx_entries}

        self.assertEqual(rx["elcon_status"].periph, "FDCAN2")
        self.assertEqual(tx["elcon_command"].periph, "FDCAN2")
        self.assertIn("elcon_status", scaling_names)
        self.assertIn("elcon_command", scaling_names)
        self.assertIn("elcon_status", stale_names)
        self.assertTrue(ctx.filters.has_fdcan)

    def test_torque_vector_render_context_models_izze_decode(self):
        ctx = build_node_render_context(self.nodes["TORQUE_VECTOR"], self.context)
        rx = {entry.msg.name: entry for entry in ctx.rx_entries}

        angular = rx["IZZE_angular_rate"]
        self.assertEqual(angular.periph, "FDCAN2")
        self.assertEqual([codec.signal.name for codec in angular.codecs], ["X_axis", "Y_axis", "Z_axis", "reserved"])
        self.assertEqual(angular.codecs[0].bswap_width, 16)
        self.assertEqual(angular.codecs[0].sign_extend_shift, 48)

    def test_signal_codec_metadata_covers_core_shapes(self):
        msg = Message(
            name="codec_shapes",
            byte_order="big_endian",
            signals=[
                Signal("be_signed", "int16_t"),
                Signal("be_float", "float"),
                Signal("flag", "bool"),
            ],
        )
        msg.resolve_layout({})

        be_signed_rx = build_signal_codec(msg.signals[0], "rx")
        be_float_tx = build_signal_codec(msg.signals[1], "tx")
        flag_rx = build_signal_codec(msg.signals[2], "rx")

        self.assertEqual(be_signed_rx.bswap_width, 16)
        self.assertEqual(be_signed_rx.sign_extend_shift, 48)
        self.assertTrue(be_float_tx.is_float32)
        self.assertEqual(be_float_tx.bswap_width, 32)
        self.assertIsNone(flag_rx.bswap_width)
        self.assertIsNone(flag_rx.sign_extend_shift)


if __name__ == "__main__":
    unittest.main()

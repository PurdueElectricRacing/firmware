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
        self.assertTrue(any("__builtin_bswap16" in line for line in angular.codecs[0].rx_lines))
        self.assertTrue(any("<< (48)" in line for line in angular.codecs[0].rx_lines))

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

        be_signed_rx = build_signal_codec(msg.signals[0], "rx", msg)
        be_float_tx = build_signal_codec(msg.signals[1], "tx", msg)
        flag_rx = build_signal_codec(msg.signals[2], "rx", msg)

        self.assertTrue(any("__builtin_bswap16" in line for line in be_signed_rx.rx_lines))
        self.assertTrue(any("int64_t" in line for line in be_signed_rx.rx_lines))
        self.assertTrue(any("union { float f; uint32_t u; }" in line for line in be_float_tx.tx_lines))
        self.assertTrue(any("__builtin_bswap32" in line for line in be_float_tx.tx_lines))
        self.assertFalse(any("__builtin_bswap" in line for line in flag_rx.rx_lines))


if __name__ == "__main__":
    unittest.main()

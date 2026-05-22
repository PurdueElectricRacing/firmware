import glob
import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path

import cantools

REPO_ROOT = Path(__file__).resolve().parents[2]
CANPILER_DIR = REPO_ROOT / "can_library" / "canpiler"
sys.path.insert(0, str(CANPILER_DIR))

from parser import Message, Signal, create_system_context, load_bus_configs, load_custom_types, parse_all
from faultgen import augment_system_with_faults
from linker import link_all
from mapper import map_hardware
import dbcgen


def build_context():
    with contextlib.redirect_stdout(io.StringIO()):
        nodes = parse_all()
        bus_configs = load_bus_configs()
        custom_types = load_custom_types()
        augment_system_with_faults(nodes, bus_configs, custom_types)
        link_all(nodes)
        mappings = map_hardware(nodes, bus_configs)
    return create_system_context(nodes, mappings, bus_configs, custom_types)


class BigEndianLayoutTests(unittest.TestCase):
    def test_byte_aligned_big_endian_words_have_c_shifts_and_dbc_starts(self):
        msg = Message(
            name="sample",
            byte_order="big_endian",
            signals=[
                Signal("a", "uint16_t"),
                Signal("b", "int16_t"),
                Signal("c", "uint16_t"),
                Signal("d", "uint16_t"),
            ],
        )

        msg.resolve_layout({})

        self.assertEqual([sig.bit_shift for sig in msg.signals], [0, 16, 32, 48])
        self.assertEqual([sig.bit_offset for sig in msg.signals], [7, 23, 39, 55])
        self.assertEqual([sig.byte_order for sig in msg.signals], ["big_endian"] * 4)

    def test_elcon_style_status_flags_remain_byte_local_little_endian_bits(self):
        msg = Message(
            name="elcon_style",
            byte_order="big_endian",
            signals=[
                Signal("voltage", "uint16_t"),
                Signal("current", "uint16_t"),
                Signal("flag0", "bool"),
                Signal("flag1", "bool"),
                Signal("flag2", "bool"),
                Signal("flag3", "bool"),
                Signal("flag4", "bool"),
            ],
        )

        msg.resolve_layout({})

        self.assertEqual([sig.bit_shift for sig in msg.signals], [0, 16, 32, 33, 34, 35, 36])
        self.assertEqual([sig.bit_offset for sig in msg.signals], [7, 23, 32, 33, 34, 35, 36])
        self.assertEqual(
            [sig.byte_order for sig in msg.signals],
            ["big_endian", "big_endian", "little_endian", "little_endian", "little_endian", "little_endian", "little_endian"],
        )

    def test_non_byte_aligned_big_endian_word_is_rejected(self):
        msg = Message(
            name="bad_motorola",
            byte_order="big_endian",
            signals=[
                Signal("flag", "bool"),
                Signal("word", "uint16_t"),
            ],
        )

        with contextlib.redirect_stdout(io.StringIO()):
            with self.assertRaisesRegex(ValueError, "Unsupported big-endian signal layout"):
                msg.resolve_layout({})


class GeneratedDbcEndianTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        context = build_context()
        cls._tmp = tempfile.TemporaryDirectory()
        cls._old_dbc_dir = dbcgen.DBC_DIR
        dbcgen.DBC_DIR = Path(cls._tmp.name)
        with contextlib.redirect_stdout(io.StringIO()):
            dbcgen.generate_dbcs(context)

        vcan_paths = glob.glob(str(Path(cls._tmp.name) / "VCAN_*.dbc"))
        ccan_paths = glob.glob(str(Path(cls._tmp.name) / "CCAN_*.dbc"))
        assert len(vcan_paths) == 1
        assert len(ccan_paths) == 1
        cls.vcan = cantools.database.load_file(vcan_paths[0])
        cls.ccan = cantools.database.load_file(ccan_paths[0])

    @classmethod
    def tearDownClass(cls):
        dbcgen.DBC_DIR = cls._old_dbc_dir
        cls._tmp.cleanup()

    def test_izze_angular_rate_dbc_matches_datasheet_byte_pairs(self):
        msg = self.vcan.get_message_by_name("IZZE_angular_rate")

        self.assertEqual(msg.frame_id, 0x4EC)
        self.assertFalse(msg.is_extended_frame)
        self.assertEqual(
            [(sig.name, sig.start, sig.length, sig.byte_order, sig.is_signed) for sig in msg.signals],
            [
                ("X_axis", 7, 16, "big_endian", True),
                ("Y_axis", 23, 16, "big_endian", True),
                ("Z_axis", 39, 16, "big_endian", True),
                ("reserved", 55, 16, "big_endian", False),
            ],
        )

        decoded = msg.decode(bytes.fromhex("0001fffe7fff0000"))
        self.assertAlmostEqual(decoded["X_axis"], 0.1)
        self.assertAlmostEqual(decoded["Y_axis"], -0.2)
        self.assertAlmostEqual(decoded["Z_axis"], 3276.7)
        self.assertEqual(decoded["reserved"], 0)

    def test_izze_acceleration_dbc_matches_datasheet_byte_pairs(self):
        msg = self.vcan.get_message_by_name("IZZE_acceleration")

        self.assertEqual(msg.frame_id, 0x4ED)
        self.assertFalse(msg.is_extended_frame)
        self.assertEqual(
            [(sig.name, sig.start, sig.length, sig.byte_order, sig.is_signed) for sig in msg.signals],
            [
                ("X_axis", 7, 16, "big_endian", True),
                ("Y_axis", 23, 16, "big_endian", True),
                ("Z_axis", 39, 16, "big_endian", True),
                ("temperature", 55, 16, "big_endian", True),
            ],
        )

        decoded = msg.decode(bytes.fromhex("0001fffe7fff000a"))
        self.assertAlmostEqual(decoded["X_axis"], 0.01)
        self.assertAlmostEqual(decoded["Y_axis"], -0.02)
        self.assertAlmostEqual(decoded["Z_axis"], 327.67)
        self.assertAlmostEqual(decoded["temperature"], 1.0)

    def test_elcon_dbc_preserves_extended_ids_and_byte_order(self):
        command = self.ccan.get_message_by_name("elcon_command")
        status = self.ccan.get_message_by_name("elcon_status")

        self.assertEqual(command.frame_id, 0x1806E5F4)
        self.assertTrue(command.is_extended_frame)
        self.assertEqual(status.frame_id, 0x18FF50E5)
        self.assertTrue(status.is_extended_frame)

        command_decoded = command.decode(bytes.fromhex("0c81024601020304"), decode_choices=False)
        self.assertAlmostEqual(command_decoded["voltage_limit"], 320.1)
        self.assertAlmostEqual(command_decoded["current_limit"], 58.2)
        self.assertEqual(command_decoded["charger_disable"], 1)
        self.assertEqual(command_decoded["reserved1"], 2)
        self.assertEqual(command_decoded["reserved2"], 3)
        self.assertEqual(command_decoded["reserved3"], 4)

        status_decoded = status.decode(bytes.fromhex("0c81024615"), decode_choices=False)
        self.assertAlmostEqual(status_decoded["charge_voltage"], 320.1)
        self.assertAlmostEqual(status_decoded["charge_current"], 58.2)
        self.assertEqual(status_decoded["hw_fail"], 1)
        self.assertEqual(status_decoded["temp_fail"], 0)
        self.assertEqual(status_decoded["input_v_fail"], 1)
        self.assertEqual(status_decoded["startup_fail"], 0)
        self.assertEqual(status_decoded["communication_fail"], 1)


if __name__ == "__main__":
    unittest.main()

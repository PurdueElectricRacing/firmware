"""PHAL filter limits and shared-template coverage for both CAN architectures.

Run with: PYTHONPATH=generators python -m unittest discover -s generators/canpiler/tests
"""

from pathlib import Path
from types import SimpleNamespace
import unittest

from jinja2 import Environment, FileSystemLoader, StrictUndefined

from canpiler.codegen.hardware import map_peripheral_filters


def messages(num_std, num_ext):
    return [
        SimpleNamespace(is_extended=extended, macro_name=f"MSG_{i}")
        for i, extended in enumerate([False] * num_std + [True] * num_ext)
    ]


class FilterTests(unittest.TestCase):
    def test_bxcan_accepts_28_mixed_ids_on_either_bus(self):
        for peripheral in ("CAN1", "CAN2"):
            with self.subTest(peripheral=peripheral):
                filters = map_peripheral_filters("TEST", peripheral, messages(27, 1), False)
                self.assertEqual(len(filters.std_ids), 27)
                self.assertEqual(len(filters.ext_ids), 1)

    def test_bxcan_rejects_combined_overflow(self):
        for peripheral in ("CAN1", "CAN2"):
            with self.subTest(peripheral=peripheral):
                with self.assertRaisesRegex(ValueError, "bxCAN combined ID.*29 > 28"):
                    map_peripheral_filters("TEST", peripheral, messages(21, 8), False)

    def test_fdcan_retains_independent_limits(self):
        filters = map_peripheral_filters("TEST", "FDCAN1", messages(28, 8), False)
        self.assertEqual(len(filters.std_ids), 28)
        self.assertEqual(len(filters.ext_ids), 8)

    def test_per_type_overflow_rejected_on_both_architectures(self):
        for peripheral in ("CAN1", "FDCAN1"):
            for num_std, num_ext, kind in ((29, 0, "standard"), (0, 9, "extended")):
                with self.subTest(peripheral=peripheral, kind=kind):
                    with self.assertRaisesRegex(ValueError, f"{kind} ID filter limit"):
                        map_peripheral_filters("TEST", peripheral, messages(num_std, num_ext), False)

    def test_shared_template_uses_correct_phal_and_id_lists(self):
        template_dir = Path(__file__).resolve().parents[1] / "codegen" / "templates"
        env = Environment(loader=FileSystemLoader(template_dir), undefined=StrictUndefined)
        template = env.get_template("filter/peripheral.c.jinja")
        for peripheral, prefix in (("CAN1", "PHAL_CAN"), ("CAN2", "PHAL_CAN"),
                                   ("FDCAN1", "PHAL_FDCAN")):
            for num_std, num_ext in ((1, 1), (1, 0), (0, 1), (0, 0)):
                for accept_all in (False, True):
                    with self.subTest(peripheral=peripheral, ids=(num_std, num_ext),
                                      accept_all=accept_all):
                        filters = map_peripheral_filters(
                            "TEST", peripheral, messages(num_std, num_ext), accept_all
                        )
                        rendered = template.render(periph=peripheral, phal_prefix=prefix,
                                                   filters=filters)
                        self.assertIn(f"{peripheral}_set_filters()", rendered)
                        self.assertNotIn("->", rendered)
                        if accept_all or not (num_std or num_ext):
                            self.assertNotIn(f"{prefix}_setFilters(", rendered)
                        else:
                            self.assertIn(f"{prefix}_setFilters(", rendered)
                            self.assertEqual("sid_list[]" in rendered, bool(num_std))
                            self.assertEqual("xid_list[]" in rendered, bool(num_ext))
                            for msg in filters.std_ids + filters.ext_ids:
                                self.assertIn(f"{msg.macro_name}_MSG_ID", rendered)


if __name__ == "__main__":
    unittest.main()

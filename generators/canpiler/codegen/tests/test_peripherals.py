"""Supported peripheral configuration for CAN code generation."""

import pytest
from pydantic import ValidationError

from canpiler.codegen.contexts import build_peripheral_views
from core.declarations import BusAttachmentDeclaration


@pytest.mark.parametrize("peripheral", ["FDCAN1", "FDCAN2", "FDCAN3"])
def test_fdcan_peripherals_supported(peripheral: str) -> None:
    attachment = BusAttachmentDeclaration(peripheral=peripheral)
    views = build_peripheral_views((attachment.peripheral,))
    assert views[0].name == peripheral
    assert views[0].enqueue_func == f"CAN_enqueue_tx_{peripheral}"


@pytest.mark.parametrize("peripheral", ["CAN1", "CAN2", "FDCAN4"])
def test_unsupported_peripheral_rejected(peripheral: str) -> None:
    with pytest.raises(ValidationError, match="peripheral"):
        BusAttachmentDeclaration(peripheral=peripheral)
    with pytest.raises(ValueError, match="Unsupported CAN peripheral"):
        build_peripheral_views((peripheral,))

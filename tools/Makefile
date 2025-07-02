all: 
	@$(MAKE) -C pcie_reg_rw
	@$(MAKE) -C pcie_reg_rw install
	@$(MAKE) -C pcie_xdma_transfer 
	@$(MAKE) -C pcie_xdma_transfer install
	@$(MAKE) -C pcie_xdma_tools
	@$(MAKE) -C pcie_xdma_tools install
	@$(MAKE) -C pcie_a53_rw
	@$(MAKE) -C pcie_a53_rw install
clean:
	@$(MAKE) -C pcie_reg_rw  clean
	@$(MAKE) -C pcie_xdma_transfer clean
	@$(MAKE) -C pcie_xdma_tools clean
	@$(MAKE) -C pcie_a53_rw  clean

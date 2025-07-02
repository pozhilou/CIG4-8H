all: 
	@$(MAKE) -C xdma
	@$(MAKE) -C tools 
clean:
	@$(MAKE) -C xdma  clean
	@$(MAKE) -C tools clean

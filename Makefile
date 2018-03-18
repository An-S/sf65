.PHONY: clean All

All:
	@echo "----------Building project:[ sf65 - Debug ]----------"
	@"$(MAKE)" -f  "sf65.mk"
clean:
	@echo "----------Cleaning project:[ sf65 - Debug ]----------"
	@"$(MAKE)" -f  "sf65.mk" clean

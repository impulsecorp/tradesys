.PHONY: clean All

All:
	@echo "----------Building project:[ tradesys - Debug ]----------"
	@$(MAKE) -f  "tradesys.mk"
clean:
	@echo "----------Cleaning project:[ tradesys - Debug ]----------"
	@$(MAKE) -f  "tradesys.mk" clean

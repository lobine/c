.PHONY: bin/json
bin/json:
	-mkdir -p bin
	gcc -I. -o $@ examples/json.c

.PHONY: json
json: bin/json
	@echo "Run examples"
	./$<


.PHONY: clean
clean:
	rm bin/*


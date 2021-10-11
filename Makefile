.PHONY: bin/json
bin/json:
	-mkdir -p bin
	gcc -I./lib -o $@ examples/json.c

.PHONY: bin/%
bin/%: app/%.c
	-mkdir -p bin
	gcc -I./lib -o $@ $<

.PHONY: json
json: bin/json
	@echo "Run examples"
	./$<


.PHONY: clean
clean:
	rm bin/*


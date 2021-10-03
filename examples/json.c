#include "c.h"
#include "io.h"
#include "json.h"


typedef struct person person;
typedef struct person_array person_array;
typedef struct address address;

struct person {
	c8 *name;
	s32 age;
	f32 height;
	b32 subscribed;
	b32 member;
	address *address;
	json_array friends;
	json_array favorite_numbers;
	json_array nicknames;
	json_array favorite_food;
	json_array favorite_places;
};

struct address {
	c8  *city;
	c8  *street;
	s32 number;
};


void* decode_person(json_decoder *decoder);
json_array decode_person_array(json_decoder *decoder);
json_array decode_array_of_places(json_decoder *decoder);
void* decode_address(json_decoder *decoder);

void* decode_person(json_decoder *decoder) {
	person *p = struct_init(person);

	json_field_spec fields[] = {
		{ .name="name",       .spec={ .kind=JSON_STRING,  .target={ .string=&p->name }}},
		{ .name="age",        .spec={ .kind=JSON_INTEGER, .target={ .integer=&p->age }}},
		{ .name="height",     .spec={ .kind=JSON_FLOAT,   .target={ .real=&p->height }}},
		{ .name="subscribed", .spec={ .kind=JSON_BOOLEAN, .target={ .boolean=&p->subscribed }}},
		{ .name="member",     .spec={ .kind=JSON_BOOLEAN, .target={ .boolean=&p->member }}},
		{
			.name="address",
			.spec={
				.kind=JSON_OBJECT,
				.callback={ .object_fn=decode_address },
				.target={ .object=(void**) &p->address },
			},
		},
		{
			.name="friends",
			.spec={
				.kind=JSON_ARRAY,
				.callback={ .array_fn=decode_person_array },
				.target={ .array=&p->friends },
			}
		},
		{
			.name="favoriteNumbers",
			.spec={ 
				.kind=JSON_ARRAY,
				.callback={ .array_fn=json_decode_array_of_integer },
				.target={ .array=&p->favorite_numbers },
			},
		},
		{
			.name="nicknames",
			.spec={
				.kind=JSON_ARRAY,
				.callback={ .array_fn=json_decode_array_of_string },
				.target={ .array=&p->nicknames },
			},
		},
		{
			.name="favoriteFood",
			.spec={
				.kind=JSON_ARRAY,
				.callback={ .array_fn=json_decode_array_of_string },
				.target={ .array=&p->favorite_food },
			},
		},
		{
			.name="favoritePlaces",
			.spec={
				.kind=JSON_ARRAY,
				.callback={ .array_fn=decode_array_of_places },
				.target={ .array=&p->favorite_places },
			},
		},
	};
	if (!json_parse_object(decoder, fields, 11)) {
		return NULL;
	}
	return p;
}

void* decode_address(json_decoder *decoder) {
	address *addr = struct_init(address);

	json_field_spec fields[] = {
		{ .name="city",   .spec={ .kind=JSON_STRING,  .target={ .string=&addr->city }}},
		{ .name="street", .spec={ .kind=JSON_STRING,  .target={ .string=&addr->street }}},
		{ .name="number", .spec={ .kind=JSON_INTEGER, .target={ .integer=&addr->number }}},
	};
	json_parse_object(decoder, fields, 3);
	return addr;
}

json_array decode_person_array(json_decoder *decoder) {
	json_array persons = { .length=0, .data=NULL };

	json_array_spec array = {
		.item_size=sizeof(person*),
		.array=&persons,
		.spec={
			.kind=JSON_OBJECT,
			.callback={ .object_fn=decode_person },
		},
	};
	json_parse_array(decoder, &array);
	return persons;
}

json_array decode_array_of_places(json_decoder *decoder) {
	json_array places = { .length=0, .data=NULL };

	json_array_spec array = {
		.item_size=sizeof(json_array),
		.array=&places,
		.spec={
			.kind=JSON_ARRAY,
			.callback={ .array_fn=json_decode_array_of_float }
		},
	};
	json_parse_array(decoder, &array);
	return places;
}

void print_person(person *p);

s32 main(s32 argc, c8 *argv[]) {
	c8 *json_file = "examples/example.json";
	c8 *json_data;
	assert(io_read_file(&json_data, json_file));
	printf("JSON:\n%s\n", json_data);

	printf("---\n");
	printf("FOUND:\n");

	json_decoder *decoder = json_make_decoder(json_data);
	person *john_doe = decode_person(decoder);
	if (!john_doe) {
		printf("NULL\n");
	} else {
		print_person(john_doe);
	}


	free(json_data);

	return 0;
}

s32 indent = 0;

void out(const c8 *format, ...) {
    c8 msg[1024];
    va_list arglist;
    va_start(arglist, format);
    vsprintf(msg, format, arglist);
    va_end(arglist);
	for (s32 i = 0; i < indent; i++) printf(" ");
    printf("%s", msg);
}

void print_person(person *p) {
	out("{\n");
	indent += 2;

	out("name: %s\n", p->name);
	out("age: %d\n", p->age);
	out("height: %f\n", p->height);
	out("subscribed: %d\n", p->subscribed);
	out("member: %d\n", p->member);

	if (p->address) {
		out("address:\n");
		out("  city: %s\n", p->address->city);
		out("  street: %s\n", p->address->street);
		out("  number: %d\n", p->address->number);
	}

	out("%d favorite number(s)", p->favorite_numbers.length);
	if (p->favorite_numbers.length > 0) {
		s32 * numbers = (s32*) p->favorite_numbers.data;
		printf(": ");
		for (s32 i = 0; i < p->favorite_numbers.length; i++) {
			if (i > 0) printf(", ");
			printf("%d", numbers[i]);
		}
	}
	printf("\n");

	out("%d nickname(s)\n", p->nicknames.length);
	if (p->nicknames.length > 0) {
		c8 **names = (c8**) p->nicknames.data;
		for (s32 i = 0; i < p->nicknames.length; i++) {
			out("- %s\n", names[i]);
		}
	}

	out("%d favorite food(s)\n", p->favorite_food.length);
	if (p->favorite_food.length > 0) {
		c8 **names = (c8**) p->favorite_food.data;
		for (s32 i = 0; i < p->favorite_food.length; i++) {
			out("- %s\n", names[i]);
		}
	}
	
	out("%d favorite place(s)\n", p->favorite_places.length);
	if (p->favorite_places.length > 0) {
		json_array *places = (json_array*) p->favorite_places.data;
		for (s32 i = 0; i < p->favorite_places.length; i++) {
			json_array place = places[i];
			f32 *coords = (f32*) place.data;
			out("- %f / %f\n", coords[0], coords[1]);
		}
	}

	out("%d friend(s)\n", p->friends.length);
	for (s32 i = 0; i < p->friends.length; i++) {
		person *friend = ((person**) p->friends.data)[i];
		print_person(friend);
	}
	indent -= 2;
	out("}\n");
}

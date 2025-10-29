// generic_query_engine.c
// Demonstrates a simple iterator-based query pipeline in C
// Supports generic filter predicate function
//
// Example query simulated:
// SELECT name, phone FROM students WHERE city = 'Pune';

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_STUDENTS 5

//Table Definition
struct Student {
    char name[20];
    int age;
    long phone;
    char city[20];
};

// Table Data
struct Student students[MAX_STUDENTS] = {
    {"pramod",   17, 9999111222, "Delhi"},
    {"shiv",  20, 9961322450, "Chennai"},
    {"raghu",  22, 8779086959, "Pune"},
    {"vikram", 26, 9876543210, "Kochi"},
    {"ayesha", 18, 1234567890, "Chennai"}
};

int table_size = MAX_STUDENTS;

// Sequential Scan Operator
int table_index = 0;

void table_scan_open() {
    table_index = 0;
    printf("TableScan: opened table.\n");
}

struct Student* table_scan_next() {
    if (table_index >= table_size)
        return NULL;
    return &students[table_index++];
}

void table_scan_close() {
    printf("TableScan: closed table.\n");
    table_index = 0;
}

void filter_open(Predicate pred, void* ctx) {
    filter_state.pred = pred;
    filter_state.ctx = ctx;
    printf("Filter: opened filter.\n");
    table_scan_open();
}

struct Student* filter_next() {
    struct Student* row;
    while ((row = table_scan_next()) != NULL) {
        if (filter_state.pred(row, filter_state.ctx)) {
            return row;
        }
    }
    return NULL;
}

void filter_close() {
    printf("Filter: closed filter.\n");
    table_scan_close();
}


// Projection Operator
void projection_open(Predicate pred, void* ctx) {
    printf("Projection: opened projection (name, phone).\n");
    filter_open(pred, ctx);
}

void projection_next() {
    struct Student* row;
    while ((row = filter_next()) != NULL) {
        printf("Result -> Name: %-10s | Phone: %ld\n", row->name, row->phone);
    }
}

void projection_close() {
    printf("Projection: closed projection.\n");
    filter_close();
}


// Query Execution
int main() {
    printf("Query: SELECT name, phone FROM students WHERE city = 'Chennai';\n\n");

    // Build predicate context
    CityPredicateCtx ctx;
    strcpy(ctx.city_name, "Chennai");

    // Execute pipeline
    projection_open(city_equals, &ctx);
    projection_next();
    projection_close();

    return 0;
}

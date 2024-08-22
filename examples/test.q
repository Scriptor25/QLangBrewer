use person as struct {
    i8* name,
    i8 age
}

def person* init_person(person* p, i8* name, i8 age) {
    p!name = name
    p!age = age
    return p
}

def i32 printf(i8*, ?)

def void print_person(person* p)
    = printf("{ name=%s, age=%d }", p!name, p!age)

def i32 main(i32 argc, i8** argv) {

    def person felix
    init_person(&felix, "Felix", 18)

    def person max
    init_person(&max, "Max", 20)

    printf("felix = ")
    print_person(&felix)
    printf("\n")
    printf("max = ")
    print_person(&max)
    printf("\n")

    return 0
}

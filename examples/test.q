use person as struct {
    i8* name,
    i8 age,
}

def +person(i8* name, i8 age) {
    self.name = name
    self.age = age
}

def i32 printf(i8*, ?)

def void person:print()
    = printf("{ name = %s, age = %d }", self.name, self.age)

def i32 main(i32 argc, i8** argv) {

    def person felix = person("Felix", 18)
    def person max = person("Max", 20)

    printf("felix = ")
    felix.print()
    printf("\n")
    printf("max = ")
    max.print()
    printf("\n")

    return 0
}

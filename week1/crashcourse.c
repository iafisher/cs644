// This is a comment!
/*
 * Multi-line comments look like this.
 */

// `#include` statements are C's primitive way of importing libraries. It's primitive
// because it literally copies the contents of the file and pastes it here.
//
// In this case, we are importing the standard I/O library so we can print things.
#include <stdio.h>

// This is a simple program to represent employees at a firm and do some basic operations
// on them. Our firm will have a few different roles, which we can represent with an enum.
// Really, this isn't much different from declaring a few constants. They are represented
// as integers under the hood. But at least you get a nice label to use.
//
// You may have heard of fancy, enum-like things called algebraic data types or variant
// types. Well, forget about them. This language was designed in the 1970s; you get an
// enum of ints and the compiler won't even type-check them for you.
enum Role {
  ROLE_DEVELOPER,
  ROLE_DESIGNER,
  ROLE_MANAGER,
};

// C is a statically-typed language, meaning the type of every variable, expression,
// function argument, etc. must be known at compile time. C is a little looser with its
// type rules than modern statically-typed languages, but that's another story...
//
// This block declares a struct called `Employee` with three fields.
struct Employee {
  // This is the enum we declared just above.
  enum Role role;
  // An `int` is an integer; `unsigned` means that it must not be negative. Unlike in,
  // say, Python, integers in C are fixed-width. You might be surprised to learn that the
  // width depends on your machine! A common value for `int` on modern machines is 32
  // bits, but portable code doesn't rely on it.
  unsigned int employee_id;
  // `char*` is what C calls strings. `char` stands for 'character'. We'll talk about the
  // star later, but for now you can think of it as an array.
  const char* name;
};

// C structs correspond to classes in languages like C++ and Java, but structs are much
// simpler. You can't define methods on a struct, and you certainly can't do inheritance
// or implement an interface.
//
// I'll take this opportunity to say that C favors simplicity in language design to an
// almost fanatical degree. This doesn't necessarily mean that C is simple to _use_ --
// there are a lot of sharp edges to cut yourself on, and you'll find yourself writing a
// lot of boilerplate because of the lack of abstractions. But you also won't be spending
// a lot of time dealing with arcane interactions between complex language features, or
// wrestling with the type system. It's a double-edged sword.


// Another thing about C is that it has a lot of old-fashioned rules. One of them is that
// you have to declare types and functions before you use them. You can tell these are
// declarations because they end in a semicolon.
//
// `void` means that the function returns no value. Note that in the declaration, we do
// not have to give the parameters names.
void print_employee(struct Employee);
// Don't worry about the asterisk for now; we'll talk about it later.
void promote_employee(struct Employee*);

// `main` is the entrypoint into a C program. It takes two arguments: `argc`, the count
// of command-line arguments, and `argv`, the arguments themselves. Remember that `char*`
// stands for string, and the brackets after `argv` indicate it's an array. So it's an
// array of strings, which makes sense.
int main(int argc, char* argv[]) {
  // Here we make our own array of `Employee` structs. C structs don't have constructors,
  // but you can initialize them with this convenient syntax.
  struct Employee employees[] = {
    { .role = ROLE_DESIGNER, .employee_id = 1, .name = "Davy Designer" },
    { .role = ROLE_DEVELOPER, .employee_id = 2, .name = "Dana Developer" },
    { .role = ROLE_MANAGER, .employee_id = 3, .name = "Meredith Manager" },
  };
  // The proper way to do this would be
  //
  //   sizeof employees / sizeof *employees
  //
  // but I don't want to have to explain that, so we're hard-coding it here.
  //
  // P.S.: `size_t` is a special type used for array indices and lengths.
  //
  // P.P.S.: Statements in C always end in semicolons. (Did you notice the semicolon at
  // the end of the first statement?)
  size_t n = 3;

  // This is a for loop -- the preferred way of iterating over sequences, though it's more
  // general than that. You can see there are three parts, separated by semi-colons:
  //
  //   size_t i = 0
  //
  // This is the initializer, run once at the beginning of the loop. We set the loop
  // variable to 0.
  //
  //   i < n
  //
  // This is the loop condition, checked before each iteration of the loop (including the
  // first).
  //
  //   i++
  //
  // This statement is run at the end of each iteration. The ++ is a fancy way of incrementing
  // the index (it's equivalent to `i += 1`).
  for (size_t i = 0; i < n; i++) {
    // This is how you call a function. Note also the syntax for indexing an array, which
    // is probably familiar to you.
    print_employee(employees[i]);

    // An if statement. Self-explanatory. You can do `else if` and `else` in the expected
    // way. The conditions must be parenthesized.
    if (i != n - 1) {
      // `puts` is one way of printing. It stands for 'put string'. We'll see `printf`
      // below, which is more versatile.
      puts("");
    }
  }

  // You should understand everything in the following line, except why is there an
  // ampersand before `employees[0]`?
  //
  // The ampersand creates a _pointer_ to a value. Normally, when you pass a value to a
  // function, C makes a copy, so any changes the function makes to the value are lost.
  // But if you pass a pointer, the function can use it to make changes to the original
  // value. Since `promote_employee` is intended to mutate its argument, that's what we
  // want here.
  promote_employee(&employees[0]);
  print_employee(employees[0]);

  // Bonus advanced topic: pointers and arrays!
  //
  // The two lines below are equivalent to the two previous lines:
  //
  //   promote_employee(employees);
  //   print_employee(*employees);
  //
  // But isn't `employees` an array, not a pointer to an individual employee? Yes, but C
  // represents arrays as pointers to the initial element. That's why `employees` is
  // equivalent to `&employees[0]`.
  //
  // This is a subtle point and it's not vital that you understand it fully right now.
  // Just remember that arrays are represented by pointers in C.


  // Returning a non-zero value from `main` indicates an error. Nothing went wrong, so
  // we'll return zero.
  return 0;
}

// By now it should come as no great shock to you that C will not automatically stringify
// an enum, so we have to do it ourselves.
const char* role_to_string(enum Role role) {
  // You don't have to initialize variables when you declare them in C. Nice! Except that
  // right now the variable `s` holds undefined junk and who knows what will happen if you
  // try to use it...
  const char* s;

  // This is a switch-case. It's a concise way of running different code depending on the
  // value of an expression. You could do the same thing with an `if-else if` statement.
  switch (role) {
    // Note the weird syntax: a colon instead of curly braces.
    case ROLE_DEVELOPER:
      s = "developer";
      // The `break` here is VERY important. If we omitted it, then this case would 'fall
      // through' and continuing executing the next case! Occasionally this is desirable;
      // often it is a bug.
      //
      // You can also use `break` to break out of loops.
      break;
    case ROLE_DESIGNER:
      // We could have just directly returned the string from inside the `switch-case`
      // instead of assigning to a variable and returning at the end; I only wrote it this
      // way to demonstrate the perils of the `break` statement.
      s = "designer";
      break;
    case ROLE_MANAGER:
      s = "manager";
      break;
    default:
      // This code runs if `role` doesn't match any of the explicit cases.
      s = "<unknown>";
      break;
  }

  return s;
}

// An important point is that C has pass-by-value semantics for function calls, which is
// unlike the pass-by-reference semantics in languages like Python and JavaScript.
// Concretely, any modifications you make to employee inside the function will _not_
// persist after the function returns, since you are operating on a copy of what was
// passed to you.
//
// We'll see how to write a function that makes modifications below.
void print_employee(struct Employee employee) {
  // `printf` is for formatted printing; it lets you interpolate values into the output
  // stream. Here, we use the `%d` format specifier for ints and `%s` for strings. Other
  // common ones are `%f` for floats and `%c` for characters.
  printf("ID:   %d\n", employee.employee_id);
  printf("Name: %s\n", employee.name);
  printf("Role: %s\n", role_to_string(employee.role));
}

// Here is how to write a function that modifies its parameter in a persistent way.
//
// Note the star after `struct Employee` in the signature; that means we are taking a
// pointer to a `struct Employee` value, rather than a `struct Employee` value directly.
void promote_employee(struct Employee* employee) {
  // The important part of this line is the `(*employee)` syntax. The star before the
  // variable is how you _dereference_ a pointer in C, meaning follow it to its original
  // value. Here, we do `(*employee)` to get a `struct Employee` value, and then get the
  // `employee_id` field on it.
  printf("\n\ndoing a promotion of employee %d\n", (*employee).employee_id);

  // Writing `(*employee).` is a little unwieldly, so C has a little syntactic sugar: the
  // `->` operator does the same thing.
  //
  // This is the line that mutates the employee value. Because it's a pointer, the change
  // we make here applies to the original value that was passed to us.
  employee->role = ROLE_MANAGER;
}

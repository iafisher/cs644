// This is a comment!
/*
 * Multi-line comments look like this.
 */

#include <stdio.h>

enum Role {
  ROLE_DEVELOPER,
  ROLE_DESIGNER,
  ROLE_MANAGER,
};

struct Employee {
  enum Role role;
  unsigned int employee_id;
  const char* name;
};

void print_employee(struct Employee);
void promote_employee(struct Employee*);

int main(int argc, char* argv[]) {
  struct Employee employees[] = {
    { .role = ROLE_DESIGNER, .employee_id = 1, .name = "Davy Designer" },
    { .role = ROLE_DEVELOPER, .employee_id = 2, .name = "Dana Developer" },
    { .role = ROLE_MANAGER, .employee_id = 3, .name = "Meredith Manager" },
  };
  size_t n = 3;

  for (size_t i = 0; i < n; i++) {
    print_employee(employees[i]);

    if (i != n - 1) {
      puts("");
    }
  }

  promote_employee(&employees[0]);
  print_employee(employees[0]);

  return 0;
}

const char* role_to_string(enum Role role) {
  const char* s;

  switch (role) {
    case ROLE_DEVELOPER:
      s = "developer";
      break;
    case ROLE_DESIGNER:
      s = "designer";
      break;
    case ROLE_MANAGER:
      s = "manager";
      break;
    default:
      s = "<unknown>";
      break;
  }

  return s;
}

void print_employee(struct Employee employee) {
  printf("ID:   %d\n", employee.employee_id);
  printf("Name: %s\n", employee.name);
  printf("Role: %s\n", role_to_string(employee.role));
}

void promote_employee(struct Employee* employee) {
  printf("\n\ndoing a promotion of employee %d\n", (*employee).employee_id);

  employee->role = ROLE_MANAGER;
}

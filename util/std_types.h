/*******************************************************************\

Module:

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#ifndef CPROVER_STD_TYPES_H
#define CPROVER_STD_TYPES_H

#include <assert.h>

#include <type.h>
#include <expr.h>

class bool_typet:public typet
{
public:
  bool_typet()
  {
    id(t_bool);
  }
};

class empty_typet:public typet
{
public:
  empty_typet()
  {
    id(t_empty);
  }
};

class symbol_typet:public typet
{
public:
  symbol_typet():typet(t_symbol)
  {
  }
  
  explicit symbol_typet(const irep_idt &identifier):typet(t_symbol)
  {
    set_identifier(identifier);
  }

  void set_identifier(const irep_idt &identifier)
  {
    set(a_identifier, identifier);
  }

  const irep_idt &get_identifier() const
  {
    return get(a_identifier);
  }  
};

// structs

class struct_union_typet:public typet
{
public:
  struct_union_typet()
  {
  }

  class componentt:public exprt
  {
  public:
    const irep_idt &get_name() const
    {
      return get(a_name);
    }

    void set_name(const irep_idt &name)
    {
      return set(a_name, name);
    }
  };

  typedef std::vector<componentt> componentst;
  
  const componentst &components() const
  {
    return (const componentst &)(find(a_components).get_sub());
  }
  
  componentst &components()
  {
    return (componentst &)(add(a_components).get_sub());
  }
  
  bool has_component(const irep_idt &component_name) const
  {
    return get_component(component_name).is_not_nil();
  }
  
  const componentt &get_component(
    const irep_idt &component_name) const;

  unsigned component_number(const irep_idt &component_name) const;
  typet component_type(const irep_idt &component_name) const;
};

extern inline const struct_union_typet &to_struct_union_type(const typet &type)
{
  assert(type.id()=="struct" ||
         type.id()=="union" ||
         type.id()=="class");
  return static_cast<const struct_union_typet &>(type);
}

extern inline struct_union_typet &to_struct_union_type(typet &type)
{
  assert(type.id()=="struct" ||
         type.id()=="union" ||
         type.id()=="class");
  return static_cast<struct_union_typet &>(type);
}

class struct_typet:public struct_union_typet
{
public:
  struct_typet()
  {
    id("struct");
  }
    
  bool is_prefix_of(const struct_typet &other) const;

  const componentst &methods() const
  {
    return (const componentst &)(find("methods").get_sub());
  }
     
  componentst &methods()
  {
    return (componentst &)(add("methods").get_sub());
  }
};

extern inline const struct_typet &to_struct_type(const typet &type)
{
  assert(type.id()=="struct" ||
         type.id()=="union" ||
         type.id()=="class");
  return static_cast<const struct_typet &>(type);
}

extern inline struct_typet &to_struct_type(typet &type)
{
  assert(type.id()=="struct" ||
         type.id()=="union" ||
         type.id()=="class");
  return static_cast<struct_typet &>(type);
}

class union_typet:public struct_union_typet
{
public:
  union_typet()
  {
    id("union");
  }
};

extern inline const union_typet &to_union_type(const typet &type)
{
  assert(type.id()=="union");
  return static_cast<const union_typet &>(type);
}

extern inline union_typet &to_union_type(typet &type)
{
  assert(type.id()=="union");
  return static_cast<union_typet &>(type);
}

// functions

class code_typet:public typet
{
public:
  code_typet()
  {
    id("code");
  }
  
  class argumentt:public exprt
  {
  public:
    argumentt():exprt("argument")
    {
    }
    
    argumentt(const typet &type):exprt("argument", type)
    {
    }
    
    const exprt &default_value() const
    {
      return find_expr("#default_value");
    }

    bool has_default_value() const
    {
      return default_value().is_not_nil();
    }

    exprt &default_value()
    {
      return add_expr("#default_value");
    }
    
    void set_identifier(const irep_idt &identifier)
    {
      set("#identifier", identifier);
    }

    void set_base_name(const irep_idt &name)
    {
      set("#base_name", name);
    }

    const irep_idt &get_identifier() const
    {
      return get("#identifier");
    }

    const irep_idt &get_base_name() const
    {
      return get("#base_name");
    }
  };
  
  bool has_ellipsis() const
  {
    return find("arguments").get_bool("ellipsis");
  }

  void make_ellipsis()
  {
    add("arguments").set("ellipsis", true);
  }

  typedef std::vector<argumentt> argumentst;

  const typet &return_type() const
  {
    return find_type("return_type");
  }

  typet &return_type()
  {
    return add_type("return_type");
  }

  const argumentst &arguments() const
  {
    return (const argumentst &)find("arguments").get_sub();
  }

  argumentst &arguments()
  {
    return (argumentst &)add("arguments").get_sub();
  }
};

extern inline const code_typet &to_code_type(const typet &type)
{
  assert(type.id()=="code");
  return static_cast<const code_typet &>(type);
}

extern inline code_typet &to_code_type(typet &type)
{
  assert(type.id()=="code");
  return static_cast<code_typet &>(type);
}

class array_typet:public typet
{
public:
  array_typet()
  {
    id("array");
  }
  
  const exprt &size() const
  {
    return (const exprt &)find("size");
  }
  
  exprt &size()
  {
    return (exprt &)add("size");
  }

};

extern inline const array_typet &to_array_type(const typet &type)
{
  assert(type.id()=="array");
  return static_cast<const array_typet &>(type);
}

extern inline array_typet &to_array_type(typet &type)
{
  assert(type.id()=="array");
  return static_cast<array_typet &>(type);
}

class pointer_typet:public typet
{
public:
  pointer_typet()
  {
    id("pointer");
  }

  explicit pointer_typet(const typet &_subtype)
  {
    id("pointer");
    subtype()=_subtype;
  }
};

class reference_typet:public pointer_typet
{
public:
  reference_typet()
  {
    set("#reference", true);
  }
};

bool is_reference(const typet &type);

class bv_typet:public typet
{
public:
  bv_typet()
  {
    id("bv");
  }

  explicit bv_typet(unsigned width)
  {
    id("bv");
    set_width(width);
  }

  unsigned get_width() const;

  void set_width(unsigned width)
  {
    set("width", width);
  }
};

class unsignedbv_typet:public bv_typet
{
public:
  unsignedbv_typet()
  {
    id("unsignedbv");
  }

  explicit unsignedbv_typet(unsigned width)
  {
    id("unsignedbv");
    set_width(width);
  }
};

class signedbv_typet:public bv_typet
{
public:
  signedbv_typet()
  {
    id("signedbv");
  }

  explicit signedbv_typet(unsigned width)
  {
    id("signedbv");
    set_width(width);
  }
};

class fixedbv_typet:public bv_typet
{
public:
  fixedbv_typet()
  {
    id("fixedbv");
  }

  unsigned get_fraction_bits() const
  {
    return get_width()-get_integer_bits();
  }

  unsigned get_integer_bits() const;

  void set_integer_bits(unsigned b)
  {
    set("integer_bits", b);
  }

  friend const fixedbv_typet &to_fixedbv_type(const typet &type)
  {
    assert(type.id()=="fixedbv");
    return static_cast<const fixedbv_typet &>(type);
  }
};

const fixedbv_typet &to_fixedbv_type(const typet &type);

class floatbv_typet:public bv_typet
{
public:
  floatbv_typet()
  {
    id("floatbv");
  }

  unsigned get_e() const
  {
    return get_width()-get_f();
  }

  unsigned get_f() const;

  void set_f(unsigned b)
  {
    set("f", b);
  }

  friend const floatbv_typet &to_floatbv_type(const typet &type)
  {
    assert(type.id()=="floatbv");
    return static_cast<const floatbv_typet &>(type);
  }
};

const floatbv_typet &to_floatbv_type(const typet &type);

class string_typet:public typet
{
public:
  string_typet():typet("string")
  {
  }

  friend const string_typet &to_string_type(const typet &type)
  {
    assert(type.id()=="string");
    return static_cast<const string_typet &>(type);
  }
};

const string_typet &to_string_type(const typet &type);

#endif

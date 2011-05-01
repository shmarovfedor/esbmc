/*******************************************************************\

Module:

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include "type.h"

/*******************************************************************\

Function: typet::copy_to_subtypes

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void typet::copy_to_subtypes(const typet &type)
{
  subtypes().push_back(type);
}

/*******************************************************************\

Function: typet::move_to_subtypes

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void typet::move_to_subtypes(typet &type)
{
  subtypest &sub=subtypes();
  sub.push_back(static_cast<const typet &>(get_nil_irep()));
  sub.back().swap(type);
}

/*******************************************************************\

Function: is_number

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

bool is_number(const typet &type)
{
  const std::string &id=type.id_string();
  return id=="rational" ||
         id=="real" ||
         id=="integer" ||
         id=="natural" || 
         id=="complex" ||
         id=="unsignedbv" ||
         id=="signedbv" || 
         id=="floatbv" ||
         id=="fixedbv";
}

irep_idt typet::t_integer = dstring("integer");
irep_idt typet::t_signedbv = dstring("signedbv");
irep_idt typet::t_unsignedbv = dstring("unsignedbv");
irep_idt typet::t_rational = dstring("rational");
irep_idt typet::t_real = dstring("real");
irep_idt typet::t_natural = dstring("natural");
irep_idt typet::t_complex = dstring("complex");
irep_idt typet::t_floatbv = dstring("floatbv");
irep_idt typet::t_fixedbv = dstring("fixedbv");
irep_idt typet::t_bool = dstring("bool");
irep_idt typet::t_empty = dstring("empty");
irep_idt typet::t_symbol = dstring("symbol");

irep_idt typet::a_identifier = dstring("identifier");
irep_idt typet::a_name = dstring("name");
irep_idt typet::a_components = dstring("components");

irep_idt typet::f_subtype = dstring("subtype");
irep_idt typet::f_subtypes = dstring("subtypes");
irep_idt typet::f_location = dstring("#location");

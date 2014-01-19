// Copyright (c) 2013, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
//
//     mFAST is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     mFAST is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public License
//     along with mFast.  If not, see <http://www.gnu.org/licenses/>.
//
#include "hpp_gen.h"
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

indent_t indent;

void hpp_gen::set_export_symbol(const char* symbol)
{
  export_symbol_ = symbol;
}

void hpp_gen::traverse(const mfast::group_field_instruction* inst, const char* name_suffix)
{
  header_cref_.inc_indent(2);
  header_mref_.inc_indent(2);
  codegen_base::traverse(inst, name_suffix);
  header_cref_.dec_indent(2);
  header_mref_.dec_indent(2);
}

void hpp_gen::gen_primitive (const char* cpp_type, const mfast::field_instruction* inst)
{
  std::string name(cpp_name(inst));
  header_cref_ << indent << "mfast::"<< cpp_type << "_cref get_" << name << "() const;\n";
  if (inst->optional() || inst->field_operator() != mfast::operator_constant)
  {
    header_mref_ << indent << "mfast::"<< cpp_type << "_mref set_" << name << "() const;\n";
    if (inst->optional()) {
      header_mref_ << indent << "void omit_" << name << "() const;\n";
    }
  }
}

void hpp_gen::visit(const mfast::int32_field_instruction* inst, void*)
{
  gen_primitive("int32", inst);
}

void hpp_gen::visit(const mfast::uint32_field_instruction* inst, void*)
{
  gen_primitive("uint32", inst);

}

void hpp_gen::visit(const mfast::int64_field_instruction* inst, void*)
{
  gen_primitive("int64", inst);
}

void hpp_gen::visit(const mfast::uint64_field_instruction* inst, void*)
{
  gen_primitive("uint64", inst);
}

void hpp_gen::visit(const mfast::decimal_field_instruction* inst, void*)
{
  gen_primitive("decimal", inst);
}

void hpp_gen::visit(const mfast::ascii_field_instruction* inst, void*)
{
  gen_primitive("ascii_string", inst);
}

void hpp_gen::visit(const mfast::unicode_field_instruction* inst, void*)
{
  gen_primitive("unicode_string", inst);
}

void hpp_gen::visit(const mfast::byte_vector_field_instruction* inst, void*)
{
  gen_primitive("byte_vector", inst);
}

void hpp_gen::visit(const mfast::int32_vector_field_instruction* inst, void*)
{
  gen_primitive("int32_vector", inst);
}

void hpp_gen::visit(const mfast::uint32_vector_field_instruction* inst, void*)
{
  gen_primitive("uint32_vector", inst);
}

void hpp_gen::visit(const mfast::int64_vector_field_instruction* inst, void*)
{
  gen_primitive("int64_vector", inst);
}

void hpp_gen::visit(const mfast::uint64_vector_field_instruction* inst, void*)
{
  gen_primitive("uint64_vector", inst);
}

void hpp_gen::visit(const mfast::group_field_instruction* inst, void* top_level)
{
  std::string name( cpp_name( inst ) );

  if (contains_only_templateRef(inst)) {
    std::string cpp_type = "mfast::nested_message"; // dynamic templateRef
    if (inst->ref_instruction()) {
      // this is static templateRef
      cpp_type = referenced_type_name(inst);
    }

    if (!top_level) {
      header_cref_ << indent << "typedef " << cpp_type << "_cref " << name << "_cref;\n";
      header_mref_ << indent << "typedef " << cpp_type << "_mref " << name << "_mref;\n";
      header_cref_ << indent << name << "_cref get_" << name << "() const;\n";
      header_mref_ << indent << name << "_mref set_" << name << "() const;\n";

      if (inst->optional()) {
        header_mref_ << indent << "void omit_" << name << "() const;\n";
      }
    }
  }
  else
  {

    header_cref_ << "\n"
                 << indent << "class " << name << "_cref\n"
                 << indent << "  : public mfast::aggregate_cref\n"
                 << indent << "{\n"
                 << indent << "  typedef mfast::aggregate_cref base_type;\n"
                 << indent << "  public:\n"
                 << indent << "    typedef mfast::group_field_instruction instruction_type;\n"
                 << indent << "    typedef const instruction_type* instruction_cptr;\n"
                 << indent << "    " << name << "_cref(\n"
                 << indent << "      const mfast::value_storage*   storage,\n"
                 << indent << "      instruction_cptr              instruction);\n\n"
                 << indent << "    explicit " << name << "_cref(const mfast::field_cref& other);\n\n";

    std::stringstream mref_base_type;
    mref_base_type << "mfast::make_aggregate_mref<" << cref_scope_.str() << name << "_cref>";

    header_mref_ << "\n"
                 << indent << "class " << name << "_mref\n"
                 << indent << "  : public " << mref_base_type.str() << "\n"
                 << indent << "{\n"
                 << indent << "  typedef " << mref_base_type.str() << " base_type;\n"
                 << indent << "  public:\n"
                 << indent << "    " << name << "_mref(\n"
                 << indent << "      mfast::allocator*             alloc,\n"
                 << indent << "      mfast::value_storage*         storage,\n"
                 << indent << "      instruction_cptr              instruction);\n\n"
                 << indent << "    explicit " << name << "_mref(const mfast::field_mref_base& other);\n\n";

    this->traverse(inst, "");

    header_cref_ << indent << "};\n\n";
    header_mref_ << indent << "};\n\n";
    if (!top_level) {
      header_cref_ << indent << name << "_cref get_" << name << "() const;\n";
      header_mref_ << indent << name << "_mref set_" << name << "() const;\n";
      if (inst->optional()) {
        header_mref_ << indent << "void omit_" << name << "() const;\n";
      }
    }
  }

  if (top_level)
  {
    content_<< header_cref_.str() << header_mref_.str();

    header_cref_.clear();
    header_cref_.str("");
    header_mref_.clear();
    header_mref_.str("");

    content_<< "class ";

    if (export_symbol_.size()) {
      content_ << boost::to_upper_copy(export_symbol_) << " ";
    }

    content_<< name << "\n"
            << "  : private boost::array<mfast::value_storage, " << inst->subinstructions_count() << ">\n"
            << "  , public mfast::group_type\n"
            << "{\n"
            << "  typedef mfast::group_type base_type;\n"
            << "  public:\n"
            << "    typedef mfast::group_instruction_ex<" << name << "_cref> instruction_type;\n\n"
            << "    typedef " << name << "_cref cref_type;\n"
            << "    typedef " << name << "_mref mref_type;\n"
            << "    " <<   name << "(\n"
            << "      mfast::allocator* alloc=mfast::malloc_allocator::instance());\n"
            << "    " <<   name << "(\n"
            << "      const " <<  name << "_cref& other,\n"
            << "      mfast::allocator* alloc=mfast::malloc_allocator::instance());\n"
            << "    cref_type ref() const;\n"
            << "    cref_type cref() const;\n"
            << "    mref_type ref();\n"
            << "    mref_type mref();\n"
            << "    static const instruction_type* instruction();\n"
            << "  private:\n"
            << "    " << name << "(const " << name << "&);\n"
            << "    " << name << "& operator = (const "  << name << "&);\n"
            << "};\n\n";
  }
}

void hpp_gen::visit(const mfast::sequence_field_instruction* inst, void* top_level)
{
  std::string name( cpp_name( inst ) );

  if (inst->ref_instruction()) {
    std::string cpp_type = referenced_type_name(inst);

    header_cref_ << indent << "typedef " << cpp_type << "_cref " << name << "_cref;\n";
    header_mref_ << indent << "typedef " << cpp_type << "_mref " << name << "_mref;\n";

  }
  else if (inst->subinstructions_count() > 1)
  {
    std::string element_type = name + "_element";

    header_cref_ << "\n"
                 << indent << "class " << element_type << "_cref\n"
                 << indent << "  : public mfast::sequence_element_cref\n"
                 << indent << "{\n"
                 << indent << "  typedef mfast::sequence_element_cref base_type;\n"
                 << indent << "  public:\n"
                 << indent << "    typedef mfast::sequence_instruction_ex<" << element_type << "_cref> instruction_type;\n"
                 << indent << "    typedef const instruction_type* instruction_cptr;\n"
                 << indent << "    " << element_type << "_cref(\n"
                 << indent << "      const mfast::value_storage* storage,\n"
                 << indent << "      instruction_cptr            instruction);\n\n";

    header_mref_ << "\n"
                 << indent << "class " << element_type << "_mref\n"
                 << indent << "  : public mfast::make_aggregate_mref<" << cref_scope_.str() << element_type << "_cref>\n"
                 << indent << "{\n"
                 << indent << "  typedef mfast::make_aggregate_mref<" << cref_scope_.str() << element_type << "_cref> base_type;\n"
                 << indent << "  public:\n"
                 << indent << "    " << element_type << "_mref(\n"
                 << indent << "      mfast::allocator*     alloc,\n"
                 << indent << "      mfast::value_storage* storage,\n"
                 << indent << "      instruction_cptr      instruction);\n\n";

    this->traverse(inst, "_element");

    header_cref_ << indent << "};\n\n";
    header_mref_ << indent << "};\n\n";

    header_cref_ << indent << "typedef mfast::make_sequence_cref<" << element_type << "_cref> " << name << "_cref;\n";
    header_mref_ << indent << "typedef mfast::make_sequence_mref<" << element_type << "_mref> " << name << "_mref;\n";

  }
  else {

    std::string element_type;

    if (inst->subinstruction(0)->field_type() == mfast::field_type_templateref) {
      element_type = "mfast::nested_message";
    }
    else{
      static const char* names[] = {
        "mfast::int32",
        "mfast::uint32",
        "mfast::int64",
        "mfast::uint64",
        "mfast::decimal",
        "mfast::exponent",
        "mfast::ascii_string",
        "mfast::unicode_string",
        "mfast::byte_vector",
      };
      element_type = names[inst->subinstruction(0)->field_type()];
    }

    header_cref_ << indent << "typedef mfast::make_sequence_cref<" << element_type << "_cref> " << name << "_cref;\n";
    header_mref_ << indent << "typedef mfast::make_sequence_mref<" << element_type << "_mref> " << name << "_mref;\n";
  }

  if (!top_level) {
    header_cref_ << indent << name << "_cref get_" << name << "() const;\n";
    header_mref_ << indent << name << "_mref set_" << name << "() const;\n";
    if (inst->optional())
      header_mref_ << indent << "void omit_" << name << "() const;\n";
  }

  if (top_level)
  {
    content_<< header_cref_.str() << header_mref_.str();

    header_cref_.clear();
    header_cref_.str("");
    header_mref_.clear();
    header_mref_.str("");
    content_<< "class ";

    if (export_symbol_.size()) {
      content_ << boost::to_upper_copy(export_symbol_) << " ";
    }

    content_<< name << "\n"
            << "  : public mfast::sequence_type\n"
            << "{\n"
            << "  typedef mfast::sequence_type base_type;\n"
            << "  public:\n"
            << "    typedef " << name << "_cref::instruction_type instruction_type;\n\n"
            << "    typedef " << name << "_cref cref_type;\n"
            << "    typedef " << name << "_mref mref_type;\n"
            << "    " <<   name << "(\n"
            << "      mfast::allocator* alloc=mfast::malloc_allocator::instance());\n"
            << "    " <<   name << "(\n"
            << "      const " <<  name << "_cref& other,\n"
            << "      mfast::allocator* alloc=mfast::malloc_allocator::instance());\n"
            << "    cref_type ref() const;\n"
            << "    cref_type cref() const;\n"
            << "    mref_type ref();\n"
            << "    mref_type mref();\n"
            << "    static const instruction_type* instruction();\n"
            << "  private:\n"
            << "    " << name << "(const " << name << "&);\n"
            << "    " << name << "& operator = (const "  << name << "&);\n"
            << "};\n\n";
  }
}

void hpp_gen::visit(const mfast::template_instruction* inst, void*)
{
  if (inst->subinstructions_count() == 0)
    return;

  std::string name ( cpp_name(inst) );
  header_cref_ << "\n"
               << indent << "class " << name << "_cref\n"
               << indent << "  : public mfast::aggregate_cref\n"
               << indent << "{\n"
               << indent << "  typedef mfast::aggregate_cref base_type;\n"
               << indent << "  public:\n"
               << indent << "    typedef mfast::template_instruction_ex<"<< name << "_cref> instruction_type;\n"
               << indent << "    typedef const instruction_type* instruction_cptr;\n"
               << indent << "    " << name << "_cref(\n"
               << indent << "      const mfast::value_storage* storage,\n"
               << indent << "      const instruction_type*     instruction);\n\n"
               << indent << "    " << name << "_cref(const mfast::message_cref& other);\n\n"
               << indent << "    explicit " << name << "_cref(const mfast::field_cref& other);\n\n";

  header_mref_ << "\n"
               << indent << "typedef mfast::make_aggregate_mref<" << name << "_cref> " << name << "_mref_base;\n"
               << indent << "class " << name << "_mref\n"
               << indent << "  : public " << name << "_mref_base\n"
               << indent << "{\n"
               << indent << "  typedef " << name << "_mref_base base_type;\n"
               << indent << "  public:\n"
               << indent << "    typedef mfast::template_instruction_ex<"<< name << "_cref> instruction_type;\n"
               << indent << "    typedef const instruction_type* instruction_cptr;\n"
               << indent << "    " << name << "_mref(\n"
               << indent << "      mfast::allocator*       alloc,\n"
               << indent << "      mfast::value_storage*   storage,\n"
               << indent << "      const instruction_type* instruction);\n\n"
               << indent << "   " << name << "_mref(const mfast::message_mref& other);\n\n"
               << indent << "    explicit " << name << "_mref(const mfast::field_mref_base& other);\n\n";


  this->traverse(inst, "");

  header_cref_ << "\n"
               << indent << "};\n\n";
  header_mref_ << indent << "};\n\n";

  content_<< header_cref_.str() << header_mref_.str();

  header_cref_.clear();
  header_cref_.str("");
  header_mref_.clear();
  header_mref_.str("");

  content_<< "class ";

  if (export_symbol_.size()) {
    content_ << boost::to_upper_copy(export_symbol_) << " ";
  }

  content_<< name << "\n"
          << "  : private boost::array<mfast::value_storage, " << inst->subinstructions_count() << ">\n"
          << "  , public mfast::message_type\n"
          << "{\n"
          << "  typedef mfast::message_type base_type;\n"
          << "  public:\n"
          << "    enum {\n"
          << "      the_id = " << inst->id() << "\n"
          << "    };\n"
          << "    typedef mfast::template_instruction_ex<" << name << "_cref> instruction_type;\n\n"
          << "    typedef mfast::make_message_cref<" << name << "_cref, instruction_type> cref_type;\n"
          << "    typedef mfast::make_message_mref<" << name << "_mref, instruction_type> mref_type;\n"
          << "    " <<   name << "(\n"
          << "      mfast::allocator* alloc=mfast::malloc_allocator::instance());\n"
          << "    " <<   name << "(\n"
          << "      const " <<  name << "_cref& other,\n"
          << "      mfast::allocator* alloc=mfast::malloc_allocator::instance());\n"
          << "    cref_type ref() const;\n"
          << "    cref_type cref() const;\n"
          << "    mref_type ref();\n"
          << "    mref_type mref();\n"
          << "    static const instruction_type* instruction();\n"
          << "  private:\n"
          << "    " << name << "(const " << name << "&);\n"
          << "    " << name << "& operator = (const "  << name << "&);\n"
          << "};\n\n";

}

void hpp_gen::visit(const mfast::templateref_instruction* inst, void*)
{
  header_cref_ << indent << "mfast::nested_message_cref get_nested_message" << inst->field_index() << "() const;\n";
  header_mref_ << indent << "mfast::nested_message_mref set_nested_message" << inst->field_index() << "() const;\n";
}

void hpp_gen::generate(mfast::dynamic_templates_description& desc)
{
  codegen_base::traverse(desc);

  std::string filebase_upper = boost::to_upper_copy(filebase_);

  out_<< "#ifndef __" << filebase_upper << "_H__\n"
      << "#define __" << filebase_upper << "_H__\n"
      << "\n"
      << "#include <mfast.h>\n"
      << "#include <boost/array.hpp>\n";

  BOOST_FOREACH(const std::string& dep, dependency_)
  {
    out_ << "#include <" << dep << ".h>\n";
  }

  if (export_symbol_.size()) {
    out_ << "#include \"" << export_symbol_ << ".h\"\n";
  }

  out_ << "namespace " << filebase_ << "\n{\n"
       << content_.str()
       << "\n";

  if (export_symbol_.size()) {
    out_ << boost::to_upper_copy(export_symbol_) << " ";
  }

  out_ << "mfast::templates_description* description();\n\n"
       << "#include \"" << filebase_ << ".inl\"\n"
       << "}\n\n"
       << "#endif //__" << filebase_upper << "_H__\n";
}

void hpp_gen::visit(const mfast::enum_field_instruction* inst, void* top_level)
{
  std::string name (cpp_name(inst));

  if (inst->ref_instruction()) {
    std::string actual_type_name = referenced_type_name(inst);
    header_cref_ << indent << "typedef " << actual_type_name << "_cref " << name << "_cref;\n";
    header_mref_ << indent << "typedef " << actual_type_name << "_mref " << name << "_mref;\n";
  }
  else {
    // this is the enum definition
    header_cref_ << indent << "struct ";

    if (export_symbol_.size()) {
      out_ << boost::to_upper_copy(export_symbol_) << " ";
    }

    header_cref_ << inst->name() << "\n"
                 << indent << "{\n"
                 << indent << "  enum element {";

    for (uint64_t i = 0; i < inst->num_elements(); ++i) {
      if (i != 0)
        header_cref_ << ",";
      header_cref_ << "\n" << indent << "    " << cpp_name(inst->elements()[i]);
      if (inst->element_values()) {
        header_cref_ << " = " << inst->element_values()[i];
      }
    }

    header_cref_ << "\n" << indent << "  };\n"
                 << indent << "  typedef mfast::enum_field_instruction_ex<" << name << "> instruction_type;\n"
                 << indent << "  static const instruction_type* instruction();\n"
                 << indent << "};\n\n";


    header_cref_ << indent << "class " << name << "_cref\n"
                 << indent << "  : public mfast::enum_cref_ex<" << name << "_cref, " << name << ">\n"
                 << indent << "{\n"
                 << indent << "  public:\n"
                 << indent << "    typedef mfast::enum_cref_ex<" << name << "_cref, " << name << "> base_type;\n"
                 << indent << "    typedef " << name << "::element element_type;\n"
                 << indent << "    typedef " << name << "::instruction_type instruction_type;\n"

                 << indent << "    " << name << "_cref(\n"
                 << indent << "      const mfast::value_storage* storage=0,\n"
                 << indent << "      instruction_cptr            instruction=0);\n\n"

                 << indent << "    explicit " << name << "_cref(const field_cref& other);\n\n";


    for (uint64_t i = 0; i < inst->num_elements_; ++i) {
      std::string element_name = cpp_name(inst->elements_[i]);
      header_cref_ << indent << "    bool is_" << element_name << "() const;\n";
    }

    header_cref_ << indent << "};\n\n";


    header_mref_ << indent << "class " << name << "_mref\n"
                 << indent << "  : public mfast::enum_mref_ex<" << name << "_mref, " << name << "_cref>\n"
                 << indent << "{\n"
                 << indent << "  public:\n"
                 << indent << "    typedef  mfast::enum_mref_ex<" << name << "_mref, " << name << "_cref> base_type;\n"
                 << indent << "    typedef " << name << "::element element_type;\n"
                 << indent << "    " << name << "_mref(\n"
                 << indent << "      mfast::allocator*     alloc=0,\n"
                 << indent << "      mfast::value_storage* storage=0,\n"
                 << indent << "      instruction_cptr      instruction=0);\n"

                 << indent << "    explicit " << name << "_mref(const mfast::field_mref_base& other);\n\n";


    for (uint64_t i = 0; i < inst->num_elements_; ++i) {
      std::string element_name = cpp_name(inst->elements_[i]);
      header_mref_ << indent << "    void as_" << element_name << "() const;\n";
    }

    header_mref_ << indent << "};\n\n";
  }

  if (!top_level)
  {
    header_cref_ << indent << name << "_cref get_" << name << "() const;\n";
    if (inst->field_operator() != mfast::operator_constant)
      header_mref_ << indent << name << "_mref set_" << name << "() const;\n";
    if (inst->optional()) {
      header_mref_ << indent << "void " << " omit_" << name << "() const;\n";
    }
  }
  else {
    content_<< header_cref_.str() << header_mref_.str();

    header_cref_.clear();
    header_cref_.str("");
    header_mref_.clear();
    header_mref_.str("");
  }
}
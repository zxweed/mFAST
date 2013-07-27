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

#include <iterator>
#include <algorithm>

#include "mfast/codec_helper.h"
#include "mfast/encoder/fast_ostream.h"
#include "mfast/encoder/encoder_presence_map.h"

#include "mfast/encoder/encoder_field_operator.h"
#include "mfast/encoder/fast_ostream_inserter.h"
namespace mfast
{

void
encoder_field_operator::encode(const int32_cref& /* cref */,
                               fast_ostream& /* stream */,
                               encoder_presence_map& /* pmap */) const
{
}

void
encoder_field_operator::encode(const uint32_cref& /* cref */,
                               fast_ostream& /* stream */,
                               encoder_presence_map& /* pmap */) const
{
}

void
encoder_field_operator::encode(const int64_cref& /* cref */,
                               fast_ostream& /* stream */,
                               encoder_presence_map& /* pmap */) const
{
}

void
encoder_field_operator::encode(const uint64_cref& /* cref */,
                               fast_ostream& /* stream */,
                               encoder_presence_map& /* pmap */) const
{
}

void
encoder_field_operator::encode(const ascii_string_cref& /* cref */,
                               fast_ostream& /* stream */,
                               encoder_presence_map& /* pmap */) const
{
}

void
encoder_field_operator::encode(const unicode_string_cref& /* cref */,
                               fast_ostream& /* stream */,
                               encoder_presence_map& /* pmap */) const
{
}

void
encoder_field_operator::encode(const byte_vector_cref& /* cref */,
                               fast_ostream& /* stream */,
                               encoder_presence_map& /* pmap */) const
{
}

void
encoder_field_operator::encode(const decimal_cref& /* cref */,
                               fast_ostream& /* stream */,
                               encoder_presence_map& /* pmap */) const
{
}

namespace encoder_detail
{

template <typename Operator>
struct decimal_encoder
{
  void encode_decimal(const decimal_cref&   cref,
                      fast_ostream&         stream,
                      encoder_presence_map& pmap) const
  {
    const Operator* derived = static_cast<const Operator*>(this);
    if(!cref.has_individual_operators())
      derived->encode_impl(cref, stream, pmap);
    else  {
      derived->encode_impl(cref.for_exponent(), stream, pmap);
      if (cref.present()) {
        int64_cref mantissa_cref = cref.for_mantissa();
        encoder_field_operator* mantissa_operator = encoder_operators[mantissa_cref.instruction()->field_operator()];
        mantissa_operator->encode(mantissa_cref, stream, pmap);
      }
    }
  }

};

class no_operator
  : public encoder_field_operator
  , public mfast::detail::codec_helper
  , public decimal_encoder<no_operator>
{
  public:
    template <typename T>
    void encode_impl(const T&      cref,
                     fast_ostream& stream,
                     encoder_presence_map  & /* pmap */) const
    {
      stream << cref;

      // Fast Specification 1.1, page 22
      //
      // If a field is mandatory and has no field operator, it will not occupy any
      // bit in the presence map and its value must always appear in the stream.
      //
      // If a field is optional and has no field operator, it is encoded with a
      // nullable representation and the NULL is used to represent absence of a
      // value. It will not occupy any bits in the presence map.
      stream.save_previous_value(cref);
    }

    virtual void encode(const int32_cref&     cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const uint32_cref&    cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const int64_cref&     cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const uint64_cref&    cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const ascii_string_cref& cref,
                        fast_ostream&            stream,
                        encoder_presence_map&    pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const unicode_string_cref& cref,
                        fast_ostream&              stream,
                        encoder_presence_map&      pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const byte_vector_cref& cref,
                        fast_ostream&           stream,
                        encoder_presence_map&   pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const decimal_cref&   cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_decimal(cref, stream, pmap);
    }

};

class constant_operator
  : public encoder_field_operator
  , public mfast::detail::codec_helper
  , public decimal_encoder<constant_operator>
{
  public:
    template <typename T>
    void encode_impl(const T&              cref,
                     fast_ostream&         stream,
                     encoder_presence_map& pmap) const
    {

      // A field will not occupy any bit in the presence map if it is mandatory and has the constant operator.
      // An optional field with the constant operator will occupy a single bit. If the bit is set, the value
      // is the initial value in the instruction context. If the bit is not set, the value is considered absent.

      if (cref.optional()) {
        pmap.set_next_bit(cref.present());
        if (cref.present())
          stream.save_previous_value(cref);
      }
    }

    virtual void encode(const int32_cref&     cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const uint32_cref&    cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const int64_cref&     cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const uint64_cref&    cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const ascii_string_cref& cref,
                        fast_ostream&            stream,
                        encoder_presence_map&    pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const unicode_string_cref& cref,
                        fast_ostream&              stream,
                        encoder_presence_map&      pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const byte_vector_cref& cref,
                        fast_ostream&           stream,
                        encoder_presence_map&   pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const decimal_cref&   cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_decimal(cref, stream,pmap);
    }

};

template <typename Operation>
class copy_or_increment_operator_impl
  : public mfast::detail::codec_helper
{
  public:
    template <typename T>
    void encode_impl(const T&              cref,
                     fast_ostream&         stream,
                     encoder_presence_map& pmap) const
    {

      value_storage previous = previous_value_of(cref);
      stream.save_previous_value(cref);

      if (!previous.is_defined())
      {
        // if the previous value is undefined – the value of the field is the initial value
        // that also becomes the new previous value.
        // If the field has optional presence and no initial value, the field is considered
        // absent and the state of the previous value is changed to empty.
        if (cref.is_initial_value() || cref.absent())
        {
          pmap.set_next_bit(false);
          return;
        }
      }
      else if (previous.is_empty()) {
        // if the previous value is empty – the value of the field is empty.
        // If the field is optional the value is considered absent.
        if (cref.absent()) {
          pmap.set_next_bit(false);
          return;
        }
      }
      else if ( Operation() (cref, previous) ) {
        pmap.set_next_bit(false);
        return;
      }

      pmap.set_next_bit(true);
      stream << cref;
    }

};

struct is_same
{
  template <typename T>
  bool operator()(const int_cref<T>& v, const value_storage& prev) const
  {
    return v.absent() == prev.is_empty() && v.value() == reinterpret_cast<const T&>(prev.of_uint.content_);
  }
  
  bool operator()(const exponent_cref& v, const value_storage& prev) const
  {
    return v.absent() == prev.is_empty() && v.value() == prev.of_decimal.exponent_;
  }

  bool operator() (const decimal_cref& v, const value_storage& prev) const
  {
    return v.absent() == prev.is_empty() && v.mantissa() == prev.of_decimal.mantissa_ && v.exponent() == prev.of_decimal.exponent_;
  }

  bool operator() (const string_cref<false>& v, const value_storage& prev) const
  {
     return v.size() == prev.of_array.len_ && memcmp(v.data(),prev.of_array.content_, v.size()) == 0; return v.size() == prev.of_array.len_ && memcmp(v.data(),prev.of_array.content_, v.size()) == 0;
  }
  
  bool operator() (const string_cref<true>& v, const value_storage& prev) const
  {
     return v.size() == prev.of_array.len_ && memcmp(v.data(),prev.of_array.content_, v.size()) == 0;
  }
  
  bool operator() (const byte_vector_cref& v, const value_storage& prev) const
  {
    return v.size() == prev.of_array.len_ && memcmp(v.data(),prev.of_array.content_, v.size()) == 0;
  }

};

class copy_operator
  : public encoder_field_operator
  , public copy_or_increment_operator_impl<is_same>
  , public decimal_encoder<copy_operator>
{

  public:
    virtual void encode(const int32_cref&     cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const uint32_cref&    cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const int64_cref&     cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const uint64_cref&    cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const ascii_string_cref& cref,
                        fast_ostream&            stream,
                        encoder_presence_map&    pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const unicode_string_cref& cref,
                        fast_ostream&              stream,
                        encoder_presence_map&      pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const byte_vector_cref& cref,
                        fast_ostream&           stream,
                        encoder_presence_map&   pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const decimal_cref&   cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_decimal(cref, stream,pmap);
    }

};

struct is_increment
{
  template <typename T>
  bool operator()(const T& v, const value_storage& previous) const
  {
    return v.value() == reinterpret_cast<const typename T::value_type&>(previous.of_uint.content_) + 1;
  }

};

class increment_operator
  : public encoder_field_operator
  , copy_or_increment_operator_impl<is_increment>
{

  public:
    virtual void encode(const int32_cref&     cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const uint32_cref&    cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const int64_cref&     cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const uint64_cref&    cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

};

class default_operator
  : public encoder_field_operator
  , public mfast::detail::codec_helper
  , public decimal_encoder<default_operator>
{
  public:
    template <typename T>
    void encode_impl(const T&              cref,
                     fast_ostream&         stream,
                     encoder_presence_map& pmap) const
    {
      // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.
      // Optional integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream in a nullable representation.

      value_storage previous = previous_value_of(cref);
      stream.save_previous_value(cref);

      if (cref.has_initial_value())
      {
        //  The default operator specifies that the value of a field is either present in the stream
        //  or it will be the initial value.
        if (cref.is_initial_value()) {
          pmap.set_next_bit(false);
          return;
        }
      }
      else if (cref.absent())
      {
        // If the field has optional presence and no initial value, the field is considered absent
        // when there is no value in the stream.
        pmap.set_next_bit(false);
        return;
      }

      pmap.set_next_bit(true);
      if (is_same()(cref, previous)) {
        //  A NULL indicates that the value is absent and the state of the previous value is left unchanged.
        stream.encode_null();
      }
      else {
        stream << cref;
      }
    }

    virtual void encode(const int32_cref&     cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const uint32_cref&    cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const int64_cref&     cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const uint64_cref&    cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const ascii_string_cref& cref,
                        fast_ostream&            stream,
                        encoder_presence_map&    pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const unicode_string_cref& cref,
                        fast_ostream&              stream,
                        encoder_presence_map&      pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const byte_vector_cref& cref,
                        fast_ostream&           stream,
                        encoder_presence_map&   pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const decimal_cref&   cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      encode_decimal(cref, stream,pmap);
    }

};

class delta_operator
  : public encoder_field_operator
  , public mfast::detail::codec_helper
{
  template <typename T>
  void encode_integer(const T&      cref,
                      fast_ostream& stream) const
  {
    if (cref.absent()) {
      //  If the field has optional presence, the delta value can be NULL. In that case the value of the field is considered absent.
      stream.encode_null();
    }
    else {
      value_storage bv = delta_base_value_of( cref );
      T base(&bv, 0);

      int64_t delta = static_cast<int64_t>(cref.value() - base.value());

      stream.encode(delta, cref.instruction()->is_nullable(), false);
      stream.save_previous_value(cref);
    }

  }

  template <typename T>
  void encode_string(const T&      cref,
                     fast_ostream& stream,
                     encoder_presence_map& /* pmap */) const
  {
    if (cref.absent()) {
      stream.encode_null();
      return;
    }

    const value_storage& prev = delta_base_value_of(cref);

    T prev_cref(&prev, cref.instruction());
    typedef typename T::const_iterator const_iterator;
    typedef typename T::const_reverse_iterator const_reverse_iterator;
    typedef typename std::iterator_traits<const_iterator>::difference_type difference_type;

    std::pair<const_iterator, const_iterator> common_prefix_positions
      = std::mismatch(cref.begin(), cref.end(), prev_cref.begin());

    std::pair<const_reverse_iterator, const_reverse_iterator> common_suffix_positions
      = std::mismatch(cref.rbegin(), cref.rend(), prev_cref.rbegin());

    int32_t substraction_len;
    const_iterator delta_iterator;
    uint32_t delta_len;

    difference_type common_prefix_delta_len = cref.end() - common_prefix_positions.first;
    difference_type common_suffix_delta_len = cref.rend() - common_suffix_positions.first;

    if ( common_prefix_delta_len <= common_suffix_delta_len ) {
      substraction_len = prev_cref.end() - common_prefix_positions.second;
      delta_iterator = common_prefix_positions.first;
      delta_len = common_prefix_delta_len;
    }
    else {
      substraction_len = common_prefix_positions.second - prev_cref.end();
      delta_iterator = common_suffix_positions.first.base();
      delta_len = common_suffix_delta_len;
    }

    stream.encode(substraction_len, cref.instruction()->is_nullable(), false);
    stream.encode(delta_iterator, delta_len, false, cref.instruction());

    stream.save_previous_value(cref);
  }

  public:
    virtual void encode(const int32_cref& cref,
                        fast_ostream&     stream,
                        encoder_presence_map      & /* pmap */) const
    {
      this->encode_integer(cref, stream);
    }

    virtual void encode(const uint32_cref& cref,
                        fast_ostream&      stream,
                        encoder_presence_map       & /* pmap */) const
    {
      encode_integer(cref, stream);
    }

    virtual void encode(const int64_cref& cref,
                        fast_ostream&     stream,
                        encoder_presence_map      & /* pmap */) const
    {
      encode_integer(cref, stream);
    }

    virtual void encode(const uint64_cref& cref,
                        fast_ostream&      stream,
                        encoder_presence_map       & /* pmap */) const
    {
      encode_integer(cref, stream);
    }

    virtual void encode(const decimal_cref&   cref,
                        fast_ostream&         stream,
                        encoder_presence_map& pmap) const
    {
      if(!cref.has_individual_operators()) {

        if (cref.present()) {
          value_storage bv = delta_base_value_of(cref);

          value_storage delta_storage;
          delta_storage.of_decimal.exponent_ = cref.exponent() - bv.of_decimal.exponent_;
          delta_storage.of_decimal.mantissa_ = cref.mantissa() - bv.of_decimal.mantissa_;

          decimal_cref delta(&delta_storage, cref.instruction());
          stream << delta;

          stream.save_previous_value(cref);
        }
        else {
          stream.encode_null();
        }
      }
      else {
        encode_integer(cref.for_exponent(), stream);
        if (cref.present()) {
          int64_cref mantissa_cref = cref.for_mantissa();
          encoder_field_operator* mantissa_operator = encoder_operators[mantissa_cref.instruction()->field_operator()];
          mantissa_operator->encode(mantissa_cref, stream, pmap);
        }
      }
    }

    virtual void encode(const ascii_string_cref& cref,
                        fast_ostream&            stream,
                        encoder_presence_map&    pmap) const
    {
      encode_string(cref, stream, pmap);
    }

    virtual void encode(const unicode_string_cref& cref,
                        fast_ostream&              stream,
                        encoder_presence_map&      pmap) const
    {
      encode_string(cref, stream, pmap);
    }

    virtual void encode(const byte_vector_cref& cref,
                        fast_ostream&           stream,
                        encoder_presence_map&   pmap) const
    {
      encode_string(cref, stream, pmap);
    }

};

class tail_operator
  : public encoder_field_operator
  , public mfast::detail::codec_helper
{
  private:

    template <typename T>
    void encode_impl(const T&              cref,
                     fast_ostream&         stream,
                     encoder_presence_map& pmap) const
    {

      value_storage& prev = previous_value_of(cref);

      if (cref.absent()) {
        if (!prev.is_defined() || prev.is_empty()) {
          pmap.set_next_bit(false);
        }
        else {
          pmap.set_next_bit(true);
          stream.encode_null();
        }
      }
      else if (is_same()(cref, prev)) {
        pmap.set_next_bit(false);
      }
      else {
        pmap.set_next_bit(true);

        uint32_t tail_len;
        typedef typename T::const_iterator const_iterator;

        const_iterator tail_itr;

        value_storage base = tail_base_value_of(cref);
        T base_cref(&base, cref.instruction());

        if (cref.size() == base_cref.size()) {


          std::pair<const_iterator, const_iterator> positions
            = std::mismatch(cref.begin(), cref.end(), base_cref.begin());

          tail_itr = positions.first;
          tail_len = cref.end()-positions.first;
        }
        else {
          tail_itr = cref.begin();
          tail_len = cref.size();
        }
        stream.encode(tail_itr,
                      tail_len,
                      cref.instruction()->is_nullable(),
                      cref.instruction());
      }
      stream.save_previous_value(cref);

    }

  public:

    virtual void encode(const ascii_string_cref& cref,
                        fast_ostream&            stream,
                        encoder_presence_map&    pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const unicode_string_cref& cref,
                        fast_ostream&              stream,
                        encoder_presence_map&      pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

    virtual void encode(const byte_vector_cref& cref,
                        fast_ostream&           stream,
                        encoder_presence_map&   pmap) const
    {
      encode_impl(cref, stream, pmap);
    }

};

static no_operator no_operator_instance;
static constant_operator constant_operator_instance;
static copy_operator copy_operator_instance;
static default_operator default_operator_instance;
static delta_operator delta_operator_instance;
static increment_operator increment_operator_instance;
static tail_operator tail_operator_instance;

}

encoder_field_operator*
encoder_operators[] = {
  &encoder_detail::no_operator_instance,
  &encoder_detail::constant_operator_instance,
  &encoder_detail::delta_operator_instance,
  &encoder_detail::default_operator_instance,
  &encoder_detail::copy_operator_instance,
  &encoder_detail::increment_operator_instance,
  &encoder_detail::tail_operator_instance
};

}
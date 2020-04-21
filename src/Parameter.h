#ifndef HALIDE_PARAMETER_H
#define HALIDE_PARAMETER_H

/** \file
 * Defines the internal representation of parameters to halide piplines
 */
#include <string>

#include "IntrusivePtr.h"
#include "Type.h"
#include "Util.h"                   // for HALIDE_NO_USER_CODE_INLINE
#include "runtime/HalideRuntime.h"  // for HALIDE_ALWAYS_INLINE

namespace Halide {

struct ArgumentEstimates;
template<typename T>
class Buffer;
struct Expr;
struct Type;

namespace Internal {

struct ParameterContents;

/** A reference-counted handle to a parameter to a halide
 * pipeline. May be a scalar parameter or a buffer */
class Parameter {
    void check_defined() const;
    void check_is_buffer() const;
    void check_is_scalar() const;
    void check_dim_ok(int dim) const;
    void check_type(const Type &t) const;

protected:
    IntrusivePtr<ParameterContents> contents;

public:
    /** Construct a new undefined handle */
    Parameter() = default;

    /** Construct a new parameter of the given type. If the second
     * argument is true, this is a buffer parameter of the given
     * dimensionality, otherwise, it is a scalar parameter (and the
     * dimensionality should be zero). The parameter will be given a
     * unique auto-generated name. */
    Parameter(const Type &t, bool is_buffer, int dimensions);

    /** Construct a new parameter of the given type with name given by
     * the third argument. If the second argument is true, this is a
     * buffer parameter, otherwise, it is a scalar parameter. The
     * third argument gives the dimensionality of the buffer
     * parameter. It should be zero for scalar parameters. If the
     * fifth argument is true, the the name being passed in was
     * explicitly specified (as opposed to autogenerated). */
    Parameter(const Type &t, bool is_buffer, int dimensions, const std::string &name);

    Parameter(const Parameter &) = default;
    Parameter &operator=(const Parameter &) = default;
    Parameter(Parameter &&) = default;
    Parameter &operator=(Parameter &&) = default;

    /** Get the type of this parameter */
    Type type() const;

    /** Get the dimensionality of this parameter. Zero for scalars. */
    int dimensions() const;

    /** Get the name of this parameter */
    const std::string &name() const;

    /** Does this parameter refer to a buffer/image? */
    bool is_buffer() const;

    /** If the parameter is a scalar parameter, get its currently
     * bound value. Only relevant when jitting */
    template<typename T>
    HALIDE_NO_USER_CODE_INLINE T scalar() const {
        check_type(type_of<T>());
        return *((const T *)(scalar_address()));
    }

    /** This returns the current value of scalar<type()>()
     * as an Expr. */
    Expr scalar_expr() const;

    /** If the parameter is a scalar parameter, set its current
     * value. Only relevant when jitting */
    template<typename T>
    HALIDE_NO_USER_CODE_INLINE void set_scalar(T val) {
        check_type(type_of<T>());
        *((T *)(scalar_address())) = val;
    }

    /** If the parameter is a scalar parameter, set its current
     * value. Only relevant when jitting */
    HALIDE_NO_USER_CODE_INLINE void set_scalar(const Type &val_type, halide_scalar_value_t val) {
        check_type(val_type);
        memcpy(scalar_address(), &val, val_type.bytes());
    }

    /** If the parameter is a buffer parameter, get its currently
     * bound buffer. Only relevant when jitting */
    Buffer<void> buffer() const;

    /** Get the raw currently-bound buffer. null if unbound */
    const halide_buffer_t *raw_buffer() const;

    /** If the parameter is a buffer parameter, set its current
     * value. Only relevant when jitting */
    void set_buffer(const Buffer<void> &b);

    /** Get the pointer to the current value of the scalar
     * parameter. For a given parameter, this address will never
     * change. Only relevant when jitting. */
    void *scalar_address() const;

    /** Tests if this handle is the same as another handle */
    bool same_as(const Parameter &other) const;

    /** Tests if this handle is non-nullptr */
    bool defined() const;

    /** Get and set constraints for the min, extent, stride, and estimates on
     * the min/extent. */
    //@{
    void set_min_constraint(int dim, Expr e);
    void set_extent_constraint(int dim, Expr e);
    void set_stride_constraint(int dim, Expr e);
    void set_min_constraint_estimate(int dim, Expr min);
    void set_extent_constraint_estimate(int dim, Expr extent);
    void set_host_alignment(int bytes);
    Expr min_constraint(int dim) const;
    Expr extent_constraint(int dim) const;
    Expr stride_constraint(int dim) const;
    Expr min_constraint_estimate(int dim) const;
    Expr extent_constraint_estimate(int dim) const;
    int host_alignment() const;
    //@}

    /** Get and set constraints for scalar parameters. These are used
     * directly by Param, so they must be exported. */
    // @{
    void set_min_value(const Expr &e);
    Expr min_value() const;
    void set_max_value(const Expr &e);
    Expr max_value() const;
    void set_estimate(Expr e);
    Expr estimate() const;
    // @}

    /** Order Parameters by their IntrusivePtr so they can be used
     * to index maps. */
    bool operator<(const Parameter &other) const {
        return contents < other.contents;
    }

    /** Get the ArgumentEstimates appropriate for this Parameter. */
    ArgumentEstimates get_argument_estimates() const;
};

/** Validate arguments to a call to a func, image or imageparam. */
void check_call_arg_types(const std::string &name, std::vector<Expr> *args, int dims);

}  // namespace Internal
}  // namespace Halide

#endif

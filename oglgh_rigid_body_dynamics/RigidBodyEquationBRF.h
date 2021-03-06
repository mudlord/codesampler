#ifndef MPRIGIDBODYEQUATION_BRF_H_
#define MPRIGIDBODYEQUATION_BRF_H_

#include "mpVec3.h"
#include "mpMatrix3x3.h"

/*
 * This class defines a rigid body equation that is integrated numerically
 * using the Forward Euler integration method (the simplest first-order
 * numerical integration method possible).
 *
 * The underlying math uses two reference frames: WORLD and BODY.
 * The WORLD reference frame is an inertial (in the sense of Newtonian 
 * dynamics) reference frame, i.e. the scene coordinate system.
 * The BODY reference frame is a reference frame that is fixed with respect
 * to the rigid body formed by the principle axes of inertia.
 *
 * The numerical integration algorithm uses the Forward Euler method that is
 * a simple one-step extrapolation formula derived from the Taylor series:
 * Assuming the following ODE:
 * dy/dt = f(y,t);
 * The FWD Euler scheme extrapolates the solution as follows:
 * y_next = y_prev + dT * f(y_prev, t_prev);
 *
 *  The integration state vector includes the following quantities:
 *  - position of the center of mass expressed in the WORLD reference frame
 *  - linear momentum of the center of mass expressed in the WORLD reference frame
 *  - orientation (rotation) matrix expressed in the WORLD reference frame
 *  - angular velocity of the rigid body expressed in the BODY (principle axes
 *    of inertia to be exact) reference frame.
 * External forces must be provided in the WORLD reference frame, whereas external
 * torques must be provided in the BODY reference frame.
 *
 * The BRF suffix stands for "Body Reference Frame". It indicates that the
 * orientation matrix is integrated in the WORLD reference frame, while the
 * angular velocity vector is integrated in the BODY reference frame.
 * An alternative method is to integrate the angular momentum vector in the
 * WORLD reference frame (WRF).
 * Refer to the accompanying RigidBodyEquationWRF class for the implementation details.
 *
 * The key difference between the RigidBodyEquationWRF and RigidBodyEquationBRF
 * equations is the the former integrates angular momentum in the WORLD reference
 * frame, while the latter integrates angular velocity in the BODY reference
 * frame. 
 *
 * To do:
 * - improve the integration method. Forward Euler is cheap and simple 
 *   computationally (an advantage), but not very stable numerically,
 *   especially when equations become stiff. Consider higher order
 *   Runge-Kutta methods or Backward (Implicit) Euler instead.
 * - there is no collision detection/response. 
 * - consider replacing the orientation matrix with a quaternion.
 */

class RigidBodyEquationBRF  {
public:

    //A few useful typedefs. Replace with your own types, if needed.
    //The math is coded with an assumption that typical arithmetic
    //operators are overloaded.
    typedef mp::Vec3<double> Vec3;
    typedef mp::Matrix3x3<double> Matrix;

	RigidBodyEquationBRF();

	~RigidBodyEquationBRF();

    //performs a single integration step. 
    //The arguments are the integration time limits.
    void    integrate(double timeBegin, double timeEnd);

    //Resets the initial conditions that are used by the numerical integration.
    void    resetInitialConditions();

    //Returns total mass of the rigid body.
    double  getMass() const { return m_mass; }

    //Returns the Principle moments of inertia
    const Vec3& getPrincipleMomentsOfInertia() const { return m_principleMomentsOfInertia; }

    //Forces are specified in the WORLD reference frame.
    void    setForce(const Vec3& force)
    {
        m_force = force;
    }

    //Returns the total force that is acting on the rigid body.
    //The force is expressed in the WORLD reference frame.
    const Vec3& getForce() const { return m_force; }

    //Torques are specified in the BODY reference frame.
    //Use the convertWorldToBody method to convert a vector from the WORLD
    //reference frame to the BODY reference frame.
    void    setTorque(const Vec3& torque)
    {
        m_torque = torque;
    }

    //Returns total torque that acts on the body.
    //The torque is expressed in the BODY reference frame.
    const Vec3& getTorque() const { return m_torque; }

    //Returns position of the center of mass.
    const Vec3& getPosition() const { return m_position;  }

    //Use this method to reset position of the center of mass.
    Vec3& getPosition() { return m_position;  }

    //Returns the linear momentum.
    const Vec3& getLinearMomentum() const { return m_linearMomentum;  }

    //Use this method to reset the linear momentum vector.
    Vec3& getLinearMomentum()  { return m_linearMomentum;  }

    //Returns the orientation matrix in the WORLD reference frame.
    const Matrix& getOrientation() const { return m_orientation;  }

    //Use this method to reset the orientation matrix of the rigid body in the 
    //WORLD reference frame.
    Matrix& getOrientation()  { return m_orientation;  }

    //Returns the angular velocity vector in the BODY reference frame.
    const Vec3& getAngularVelocity() const { return m_angularVelocity; }

    //Use this method to reset the angular velocity vector
    Vec3& getAngularVelocity() { return m_angularVelocity; }

    //Converts a given vector (position) from the WORLD to the BODY
    //reference frame.
    void    convertWorldToBody(Vec3& r, bool bIsPosition) const;

    //Converts a given vector (position) from the BODY to the WORLD
    //reference frame.
    void    convertBodyToWorld(Vec3& r, bool bIsPosition) const;

    //Returns the angular momentum vector expressed in the BODY 
    //reference frame.
    void    computeAngularMomentum(Vec3& momentum) const;
    
    //Returns the cube linear dimension. This is an auxiliary method
    //that can be used by the rendering engine to visualize the body.
    double  getDimension() const { return m_dimension; }

private:

    //*************************************************************************
    //Rigid boy mass properties include the total mass
    //and the inertia tensor that represents the mass distribution.
    //*************************************************************************

    //The total mass of the rigid body.
    double  m_mass;

    //The beauty of the Principle Inertia axes is that the inertia matrix
    //is constant and diagonal in that basis. Therefore, we don't really
    //need the inertia matrix - all we need is the principle moments of inertia.
    Vec3 m_principleMomentsOfInertia;

    //Our rigid body is a cube. This is its linear dimension.
    //This is an auxiliary variable that is used by the rendering
    //engine. It is not really required by the numerical model.
    double  m_dimension;

    //*************************************************************************
    //External forces and torques define time derivatives of the 
    //Linear and the Angular momentums respectively.
    //The forces are expressed in the WORLD reference frame,
    //while the torques must be provided in the BODY reference frame.
    //*************************************************************************

    //Cumulative external force expressed in the WORLD reference frame.
    //The WORLD reference frame is assumed to be inertial.
    Vec3   m_force;

    //Cumulative external torque expressed in the BODY reference frame.
    //This vector is used when integrating the angular momentum in the WORLD
    //reference frame.
    Vec3   m_torque;

    //*************************************************************************
    //The following variables define the integration state for the center of mass.
    //The variables are expressed in the WORLD reference frame.
    //*************************************************************************

    //The following variables define the integration state
    //Position of the center of mass in the WORLD reference frame.
    Vec3   m_position;

    //Linear momentum of the rigid body, which is equal to
    //mass x velocity of the center of mass.
    //This vector is also expressed in the WORLD reference frame.
    Vec3   m_linearMomentum;

    //*************************************************************************
    //The following variables define the integration state for the orientation
    //of the rigid body. 
    //The orientation matrix is expressed in the WORLD reference frame,
    //while the angular velocity is in the BODY reference frame.
    //*************************************************************************

    //The orientation matrix expressed in the WORLD reference frame.
    //The orientation matrix is composed of the BODY reference frame unit vectors,
    //where each column of the matrix is a unit vector of the BODY reference frame
    //expressed in the WORLD reference frame. Therefore, the orientation matrix
    //is also the BODY->WORLD change of basis matrix. 
    //The matrix is orthogonal because both the WORLD and the BODY basis are 
    //right-handed and orthonormal. Therefore, its inverse is equal to its transpose.
    Matrix   m_orientation;

    //The angular velocity vector expressed in the BODY reference frame.
    Vec3   m_angularVelocity;
};
#endif //MPRIGIDBODYEQUATION_BRF_H_

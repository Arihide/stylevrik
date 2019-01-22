/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Original Author: Laurence
 * Contributor(s): Brecht
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file iksolver/intern/IK_QTask.cpp
 *  \ingroup iksolver
 */

#include "IK_QTask.h"

// IK_QTask

IK_QTask::IK_QTask(
	int size,
	bool primary,
	bool active,
	const IK_QSegment *segment) : m_size(size), m_primary(primary), m_active(active), m_segment(segment),
								  m_weight(1.0)
{
}

// IK_QPositionTask

IK_QPositionTask::IK_QPositionTask(
	bool primary,
	const IK_QSegment *segment,
	const Vector3d &goal) : IK_QTask(3, primary, true, segment), m_goal(goal)
{
	// computing clamping length
	int num;
	const IK_QSegment *seg;

	m_clamp_length = 0.0;
	num = 0;

	for (seg = m_segment; seg; seg = seg->Parent())
	{
		m_clamp_length += seg->MaxExtension();
		num++;
	}

	m_clamp_length /= 2 * num;
}

void IK_QPositionTask::ComputeJacobian(IK_QJacobian &jacobian)
{
	// compute beta
	const Vector3d &pos = m_segment->GlobalEnd();

	Vector3d d_pos = m_goal - pos;
	double length = d_pos.norm();

	// if (length > m_clamp_length)
	// 	d_pos = (m_clamp_length / length) * d_pos;

	jacobian.SetBetas(m_id, m_size, /*  m_weight *  */ d_pos);

	// compute derivatives
	int i;
	const IK_QSegment *seg;

	for (seg = m_segment; seg; seg = seg->Parent())
	{
		Vector3d p = seg->GlobalStart() - pos;

		for (i = 0; i < seg->NumberOfDoF(); i++)
		{
			Vector3d axis = seg->Axis(i) /* * m_weight */;

			if (seg->Translational())
				jacobian.SetDerivatives(m_id, seg->DoFId() + i, axis, 1e2);
			else
			{
				Vector3d pa = p.cross(axis);
				jacobian.SetDerivatives(m_id, seg->DoFId() + i, pa, 1e0);
			}
		}
	}
}

void IK_QPositionTask::ComputeExpMapJacobian(IK_QJacobian &jacobian, const VectorXd &expmap)
{

	const Vector3d &pos = m_segment->GlobalEnd();

	Vector3d d_pos = m_goal - pos;

	jacobian.SetBetas(m_id, m_size, /*  m_weight *  */ d_pos);

	const IK_QSegment *seg;

	Matrix3d wmat;
	Vector3d p = Vector3d::Zero();

	for (seg = m_segment; seg; seg = seg->Parent())
	{

		if(seg->Parent())
			wmat = seg->Parent()->GlobalTransform().linear();
		else
			wmat.setIdentity();

		Vector3d ex = expmap.segment<3>(seg->DoFId());
		double sqtheta = ex.squaredNorm();

		p += seg->m_translation;

		Matrix3d ex_hat;
		ex_hat << 0, -ex(2), ex(1),
			ex(2), 0, -ex(0),
			-ex(1), ex(0), 0;

		Matrix3d p_hat;
		p_hat << 0, -p(2), p(1),
			p(2), 0, -p(0),
			-p(1), p(0), 0;

		jacobian.m_jacobian.block<3, 3>(0, seg->DoFId()) =
			wmat * -seg->m_basis * p_hat *
			(ex * ex.transpose() + (seg->m_basis.transpose() - Matrix3d::Identity()) * ex_hat) / sqtheta;

		p = seg->m_basis * p;

	}
}

double IK_QPositionTask::Distance() const
{
	const Vector3d &pos = m_segment->GlobalEnd();
	Vector3d d_pos = m_goal - pos;
	return d_pos.norm();
}

// IK_QVelocityTask

IK_QVelocityTask::IK_QVelocityTask(
	bool primary,
	const IK_QSegment *segment,
	const Vector3d &goal) : IK_QTask(3, primary, true, segment), m_goal(goal)
{
}

void IK_QVelocityTask::ComputeJacobian(IK_QJacobian &jacobian)
{
}

void IK_QVelocityTask::ComputeExpMapJacobian(IK_QJacobian &jacobian, const VectorXd &vel)
{

	Vector3d d_vel = m_goal - vel;

	jacobian.m_jacobian.block<3, 3>(3, 9) = Matrix3d::Identity();

	jacobian.SetBetas(m_id, m_size, d_vel);

	// 以下、角速度で求めようとしていた時のコードの断片
	// const Vector3d &pos = m_segment->GlobalEnd();

	// Vector3d vel = Vector3d::Zero(3);
	// const IK_QSegment *seg;

	// int i = 3;
	// for (seg = m_segment; seg; seg = seg->Parent())
	// {
	// 	Matrix3d omega_hat;

	// 	omega_hat << 0, -angvel(2 + i + 6), angvel(1 + i + 6),
	// 		angvel(2 + i + 6), 0, -angvel(0 + i + 6),
	// 		-angvel(1 + i + 6), angvel(0 + i + 6), 0;

	// 	jacobian.m_jacobian.block<3, 3>(0, i + 6) = omega_hat * jacobian.m_jacobian.block<3, 3>(0, i);

	// 	i = 0;
	// }

	// i = 3;
	// for (seg = m_segment; seg; seg = seg->Parent())
	// {
	// 	Vector3d r = pos - seg->GlobalStart();

	// 	Matrix3d r_hat;
	// 	r_hat << 0, -r(2), r(1),
	// 		r(2), 0, -r(0),
	// 		-r(1), r(0), 0;

	// 	// jacobian.m_jacobian.block<3, 3>(0, seg->DoFId()) = r_hat;
	// 	jacobian.m_jacobian.block<3, 3>(3, i + 6) = r_hat;

	// 	vel += r_hat * angvel.segment(i + 6, 3);

	// 	i = 0;
	// }

	// Vector3d g = m_goal - vel;
	// jacobian.SetBetas(m_id, m_size, g);
}

double IK_QVelocityTask::Distance() const
{

	return 0.0;
}

// IK_QOrientationTask

IK_QOrientationTask::IK_QOrientationTask(
	bool primary,
	const IK_QSegment *segment,
	const Matrix3d &goal) : IK_QTask(3, primary, true, segment), m_goal(goal), m_distance(0.0)
{
}

void IK_QOrientationTask::ComputeJacobian(IK_QJacobian &jacobian)
{
	// compute betas
	const Matrix3d &rot = m_segment->GlobalTransform().linear();

	Matrix3d d_rotm = (m_goal * rot.transpose()).transpose();

	Vector3d d_rot;
	d_rot = -0.5 * Vector3d(d_rotm(2, 1) - d_rotm(1, 2),
							d_rotm(0, 2) - d_rotm(2, 0),
							d_rotm(1, 0) - d_rotm(0, 1));

	m_distance = d_rot.norm();

	jacobian.SetBetas(m_id, m_size, m_weight * d_rot);

	// compute derivatives
	int i;
	const IK_QSegment *seg;

	for (seg = m_segment; seg; seg = seg->Parent())
		for (i = 0; i < seg->NumberOfDoF(); i++)
		{

			if (seg->Translational())
				jacobian.SetDerivatives(m_id, seg->DoFId() + i, Vector3d(0, 0, 0), 1e2);
			else
			{
				Vector3d axis = seg->Axis(i) * m_weight;
				jacobian.SetDerivatives(m_id, seg->DoFId() + i, axis, 1e0);
			}
		}
}

// IK_QCenterOfMassTask
// Note: implementation not finished!

IK_QCenterOfMassTask::IK_QCenterOfMassTask(
	bool primary,
	const IK_QSegment *segment,
	const Vector3d &goal_center) : IK_QTask(3, primary, true, segment), m_goal_center(goal_center)
{
	m_total_mass_inv = ComputeTotalMass(m_segment);
	if (!FuzzyZero(m_total_mass_inv))
		m_total_mass_inv = 1.0 / m_total_mass_inv;
}

double IK_QCenterOfMassTask::ComputeTotalMass(const IK_QSegment *segment)
{
	double mass = /*seg->Mass()*/ 1.0;

	const IK_QSegment *seg;
	for (seg = segment->Child(); seg; seg = seg->Sibling())
		mass += ComputeTotalMass(seg);

	return mass;
}

Vector3d IK_QCenterOfMassTask::ComputeCenter(const IK_QSegment *segment)
{
	Vector3d center = /*seg->Mass()**/ segment->GlobalStart();

	const IK_QSegment *seg;
	for (seg = segment->Child(); seg; seg = seg->Sibling())
		center += ComputeCenter(seg);

	return center;
}

void IK_QCenterOfMassTask::JacobianSegment(IK_QJacobian &jacobian, Vector3d &center, const IK_QSegment *segment)
{
	int i;
	Vector3d p = center - segment->GlobalStart();

	for (i = 0; i < segment->NumberOfDoF(); i++)
	{
		Vector3d axis = segment->Axis(i) * m_weight;
		axis *= /*segment->Mass()**/ m_total_mass_inv;

		if (segment->Translational())
			jacobian.SetDerivatives(m_id, segment->DoFId() + i, axis, 1e2);
		else
		{
			Vector3d pa = axis.cross(p);
			jacobian.SetDerivatives(m_id, segment->DoFId() + i, pa, 1e0);
		}
	}

	const IK_QSegment *seg;
	for (seg = segment->Child(); seg; seg = seg->Sibling())
		JacobianSegment(jacobian, center, seg);
}

void IK_QCenterOfMassTask::ComputeJacobian(IK_QJacobian &jacobian)
{
	Vector3d center = ComputeCenter(m_segment) * m_total_mass_inv;

	// compute beta
	Vector3d d_pos = m_goal_center - center;

	m_distance = d_pos.norm();

#if 0
	if (m_distance > m_clamp_length)
		d_pos = (m_clamp_length / m_distance) * d_pos;
#endif

	jacobian.SetBetas(m_id, m_size, m_weight * d_pos);

	// compute derivatives
	JacobianSegment(jacobian, center, m_segment);
}

double IK_QCenterOfMassTask::Distance() const
{
	return m_distance;
}

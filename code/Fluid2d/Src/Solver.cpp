#include "Solver.h"
#include "Global.h"
#include <iostream>
#include <algorithm>

namespace Fluid2d {

	Solver::Solver(ParticalSystem& ps) : mPs(ps), mW(ps.mSupportRadius)
	{

	}

	Solver::~Solver() {

	}

	void Solver::Iterate() {
		UpdateDensityAndPressure();
		InitAccleration();
		UpdateViscosityAccleration();
		UpdatePressureAccleration();
		EulerIntegrate();
		BoundaryCondition();
	}

	void Solver::UpdateDensityAndPressure() {
		mPs.mDensity = std::vector<float>(mPs.mPositions.size(), Glb::density0);
		mPs.mPressure = std::vector<float>(mPs.mPositions.size(), 0.0f);
		for (int i = 0; i < mPs.mPositions.size(); i++) {	// ����������
			if (mPs.mNeighbors.size() != 0) {	// ���ھ�
				std::vector<NeighborInfo>& neighbors = mPs.mNeighbors[i];
				float density = 0;
				for (auto& nInfo : neighbors) {
					density += mW.Value(nInfo.distance);
				}
				density *= (mPs.mVolume * Glb::density0);
				mPs.mDensity[i] = density;
				//mPs.mDensity[i] = std::max(density, Glb::density0);		// ��ֹ����
			}

			// ����ѹǿ
			//mPs.mPressure[i] = mPs.mStiffness* (std::powf(mPs.mDensity[i] / Glb::density0, mPs.mExponent) - 1.0f);
		}
		//int p = 0;
		//for (int i = 0; i < 60; i++) {
		//	for (int j = 0; j < 60; j++) {
		//		std::cout << mPs.mDensity[p] << " ";
		//		p++;
		//	}
		//	std::cout << std::endl;
		//}
	}

	void Solver::InitAccleration() {
		mPs.mAccleration = std::vector<glm::vec2>(mPs.mPositions.size(), glm::vec2(0.0f, -Glb::gravity));
		//mPs.mAccleration = std::vector<glm::vec2>(mPs.mPositions.size(), glm::vec2(0.0f, 0.0f));
	}


	void Solver::UpdateViscosityAccleration() {
		float dim = 2.0f;
		float constFactor = 2.0f * (dim + 2.0f) * mPs.mViscosity;
		for (int i = 0; i < mPs.mPositions.size(); i++) {	// ����������
			if (mPs.mNeighbors.size() != 0) {	// ���ھ�
				std::vector<NeighborInfo>& neighbors = mPs.mNeighbors[i];
				glm::vec2 viscosityForce(0.0f, 0.0f);
				for (auto& nInfo : neighbors) {
					int j = nInfo.index;
					float dotDvToRad = glm::dot(mPs.mVelocity[i] - mPs.mVelocity[j], nInfo.radius);

					float denom = nInfo.distance * nInfo.distance + 0.01f * mPs.mSupportRadius2;
					viscosityForce += (mPs.mMass / mPs.mDensity[j]) * dotDvToRad * mW.Grad(nInfo.radius) / denom;
				}
				viscosityForce *= constFactor;
				mPs.mAccleration[i] += viscosityForce;
			}
		}

		//int p = 0;
		//for (int i = 0; i < 60; i++) {
		//	for (int j = 0; j < 60; j++) {
		//		std::cout << "(" << mPs.mAccleration[p].x << "," << mPs.mAccleration[p].y << ") ";
		//		p++;
		//	}
		//	std::cout << std::endl;
		//}

	}

	void Solver::UpdatePressureAccleration() {
		for (int i = 0; i < mPs.mPositions.size(); i++) {	// ����������
			mPs.mDensity[i] = std::max(mPs.mDensity[i], Glb::density0);
			mPs.mPressure[i] = mPs.mStiffness * (std::powf(mPs.mDensity[i] / Glb::density0, mPs.mExponent) - 1.0f);
		}


		for (int i = 0; i < mPs.mPositions.size(); i++) {	// ����������
			if (mPs.mNeighbors.size() != 0) {	// ���ھ�
				std::vector<NeighborInfo>& neighbors = mPs.mNeighbors[i];
				glm::vec2 pressureForce(0.0f, 0.0f);
				for (auto& nInfo : neighbors) {
					int j = nInfo.index;
					pressureForce += mPs.mDensity[j] * mPs.mVolume *
						(mPs.mPressure[i] / std::powf(mPs.mDensity[i], 2) + mPs.mPressure[j] / std::powf(mPs.mDensity[j], 2)) *
						mW.Grad(nInfo.radius);
				}
				mPs.mAccleration[i] -= pressureForce;
			}
		}

		//int p = 0;
		//float mx = 0;
		//float my = 0;
		//for (int i = 0; i < 60; i++) {
		//	for (int j = 0; j < 60; j++) {
		//		mx = std::max(mx, abs(mPs.mAccleration[p].x));
		//		my = std::max(my, abs(mPs.mAccleration[p].y));
		//		std::cout << "(" << mPs.mAccleration[p].x << "," << mPs.mAccleration[p].y << ") ";
		//		p++;
		//	}
		//	std::cout << std::endl;
		//}
		//std::cout << mx << " " << my << "...."  << std::endl;
	}

	void Solver::EulerIntegrate() {
		for (int i = 0; i < mPs.mPositions.size(); i++) {	// ����������
			mPs.mVelocity[i] += Glb::dt * mPs.mAccleration[i];
			mPs.mVelocity[i] = glm::clamp(mPs.mVelocity[i], glm::vec2(-100.0f), glm::vec2(100.0f));
			mPs.mPositions[i] += Glb::dt * mPs.mVelocity[i];
		}
	}

	void Solver::BoundaryCondition() {
		for (int i = 0; i < mPs.mPositions.size(); i++) {	// ����������
			glm::vec2& position = mPs.mPositions[i];
			if (position.x < mPs.mLowerBound.x + 0.025f ||
				position.y < mPs.mLowerBound.y + 0.025f ||
				position.x > mPs.mUpperBound.x - 0.025f ||
				position.y > mPs.mUpperBound.x - 0.025f) {
				mPs.mVelocity[i] = -1.0f * mPs.mVelocity[i];
				mPs.mPositions[i] += Glb::dt * mPs.mVelocity[i];

				mPs.mVelocity[i] = glm::clamp(mPs.mVelocity[i], glm::vec2(-100.0f), glm::vec2(100.0f));
			}

		}
	}

}


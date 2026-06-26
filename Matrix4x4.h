#pragma once
#include <cmath>
#include "Vector3D.h"

class Matrix4x4
{

public:
	Matrix4x4()
	{
	}

	void setIdentity()
	{
		::memset(m_mat, 0,  sizeof(float) * 16);
		m_mat[0][0] = 1;
		m_mat[1][1] = 1;
		m_mat[2][2] = 1;
		m_mat[3][3] = 1;
	}

	void setTranslation(const Vector3D& translation)
	{
		m_mat[3][0] = translation.m_x;
		m_mat[3][1] = translation.m_y;
		m_mat[3][2] = translation.m_z;
	}

	void setOrthoLH(float width, float height, float near_plane, float far_plane)
	{
		setIdentity();
		m_mat[0][0] = 2.0f / width;
		m_mat[1][1] = 2.0f / height;
		m_mat[2][2] = 1.0f / (far_plane - near_plane);
		m_mat[3][2] = -(near_plane / (far_plane - near_plane));
	}

	void setPerspectiveFovLH(float fov, float aspect, float near_plane, float far_plane)
	{
		setIdentity();
		float yScale = 1.0f / tan(fov / 2.0f);
		float xScale = yScale / aspect;
		m_mat[0][0] = xScale;
		m_mat[1][1] = yScale;
		m_mat[2][2] = far_plane / (far_plane - near_plane);
		m_mat[2][3] = 1.0f;
		m_mat[3][2] = (-near_plane * far_plane) / (far_plane - near_plane);
		m_mat[3][3] = 0.0f;
	}

	void setRotationX(float x)
	{
		setIdentity();
		m_mat[1][1] = cos(x);
		m_mat[1][2] = sin(x);
		m_mat[2][1] = -sin(x);
		m_mat[2][2] = cos(x);
	}

	void setRotationY(float y)
	{
		setIdentity();
		m_mat[0][0] = cos(y);
		m_mat[0][2] = -sin(y);
		m_mat[2][0] = sin(y);
		m_mat[2][2] = cos(y);
	}

	void setRotationZ(float z)
	{
		setIdentity();
		m_mat[0][0] = cos(z);
		m_mat[0][1] = sin(z);
		m_mat[1][0] = -sin(z);
		m_mat[1][1] = cos(z);
	}

	void operator *=(const Matrix4x4& matrix)
	{
		Matrix4x4 out;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				out.m_mat[i][j] =
					m_mat[i][0] * matrix.m_mat[0][j] +
					m_mat[i][1] * matrix.m_mat[1][j] +
					m_mat[i][2] * matrix.m_mat[2][j] +
					m_mat[i][3] * matrix.m_mat[3][j];
			}
		}
		::memcpy(m_mat, out.m_mat, sizeof(float) * 16);
	}

	// Returns the determinant of this 4x4 matrix using cofactor expansion along the first row
	float getDeterminant() const
	{
		float det = 0.0f;

		// 3x3 minor determinants for each cofactor along row 0
		for (int col = 0; col < 4; col++)
		{
			// Build the 3x3 minor by skipping row 0 and current col
			float minor[3][3] = {};
			int mi = 0;
			for (int r = 1; r < 4; r++)
			{
				int mj = 0;
				for (int c = 0; c < 4; c++)
				{
					if (c == col) continue;
					minor[mi][mj++] = m_mat[r][c];
				}
				mi++;
			}

			// 3x3 determinant of the minor
			float det3 =
				minor[0][0] * (minor[1][1] * minor[2][2] - minor[1][2] * minor[2][1]) -
				minor[0][1] * (minor[1][0] * minor[2][2] - minor[1][2] * minor[2][0]) +
				minor[0][2] * (minor[1][0] * minor[2][1] - minor[1][1] * minor[2][0]);

			// Alternating sign for cofactor
			float sign = (col % 2 == 0) ? 1.0f : -1.0f;
			det += sign * m_mat[0][col] * det3;
		}

		return det;
	}

	// Sets this matrix to the inverse of the input matrix
	// Uses the classical adjugate method: inv(M) = adj(M) / det(M)
	void setInverse(const Matrix4x4& matrix)
	{
		float det = matrix.getDeterminant();

		// A singular matrix has no inverse — leave unchanged
		if (fabsf(det) < 1e-6f)
			return;

		float invDet = 1.0f / det;

		// Compute the full matrix of cofactors, then transpose for the adjugate
		for (int row = 0; row < 4; row++)
		{
			for (int col = 0; col < 4; col++)
			{
				// Build 3x3 minor by skipping current row and col
				float minor[3][3] = {};
				int mi = 0;
				for (int r = 0; r < 4; r++)
				{
					if (r == row) continue;
					int mj = 0;
					for (int c = 0; c < 4; c++)
					{
						if (c == col) continue;
						minor[mi][mj++] = matrix.m_mat[r][c];
					}
					mi++;
				}

				float det3 =
					minor[0][0] * (minor[1][1] * minor[2][2] - minor[1][2] * minor[2][1]) -
					minor[0][1] * (minor[1][0] * minor[2][2] - minor[1][2] * minor[2][0]) +
					minor[0][2] * (minor[1][0] * minor[2][1] - minor[1][1] * minor[2][0]);

				float sign = ((row + col) % 2 == 0) ? 1.0f : -1.0f;

				// Transpose (col,row) ← cofactor(row,col) to get the adjugate
				m_mat[col][row] = sign * det3 * invDet;
			}
		}
	}

	~Matrix4x4()
	{
	}
public:
	float m_mat[4][4] = {};
};
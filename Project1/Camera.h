/*
Provided from University of Edinburgh as part of coursework
*/

#ifndef CAMERA_H
#define CAMERA_H

#include "Vector.h"

class Camera {

    public:

        Camera();
        Camera(Vector3 position, Vector3 target);
		Camera(Vector3 position, Vector3 target, float FOV, float near, float far);
        ~Camera();

        void SetPosition(Vector3 position);
		void SetPosition_Sperical(float radius, float azimuth, float polar);
		Vector3 GetPosition();

        void SetTarget(Vector3 target);
        Vector3 GetTarget();

        void SetDirection(Vector3 direction);
        Vector3 GetDirection();

        void SetFOV(float fov);
        float GetFOV();

        void SetNearClipPlane(float near_clip);
        float GetNearClipPlane();

        void SetFarClipPlane(float far_clip);
        float GetFarClipPlane();

        Vector3 PositionToTarget();

    private:

        Vector3 m_position;
        Vector3 m_target;

        float m_fov;

        float m_near_clip;
        float m_far_clip;

};


#endif
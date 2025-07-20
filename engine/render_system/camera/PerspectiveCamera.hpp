#pragma once

#include "BedrockCommon.hpp"
#include "BedrockRotation.hpp"
#include "Transform.hpp"

#include <functional>
#include <vulkan/vulkan.h>

namespace MFA
{
	// TODO: Camera can use transform class
	class PerspectiveCamera
	{
	public:

	    // Returns size of the window for the camera.
	    using WindowExtendCallback = std::function<VkExtent2D()>;
	    // Indicates if the window belonging to the camera is focuses or not.
	    using HasFocusCallback = std::function<bool()>;

		explicit PerspectiveCamera(WindowExtendCallback windowExtendCallback, HasFocusCallback hasFocusCallback);
		virtual ~PerspectiveCamera();

		virtual void Update(float dtSec) {}

		virtual void Debug_UI();

		[[nodiscard]]
		glm::mat4 const & ViewProjection();

		[[nodiscard]]
		glm::mat4 const & View();

		glm::mat4 const & Projection();

		void SetLocalPosition(glm::vec3 const & localPosition);

		void SetLocalRotation(Rotation const & localRotation);

		void SetEulerAngles(glm::vec3 const & eulerAngles);

		void SetLocalQuaternion(glm::quat const & quaternion);

		[[nodiscard]]
		glm::vec3 const & LocalPosition();

		[[nodiscard]]
		Rotation const & LocalRotation();

		[[nodiscard]]
		glm::vec3 const & Forward();

		[[nodiscard]]
		glm::vec3 const & Right();

		[[nodiscard]]
		glm::vec3 const & Up();

		[[nodiscard]]
		glm::vec3 const & GlobalPosition();

		[[nodiscard]]
		Rotation const & GlobalRotation();

		[[nodiscard]]
		bool IsDirty() const;

	    // Must be called when a resize even happens
	    void SetProjectionDirty();

	protected:

		void SetViewDirty();

		MFA_VARIABLE2(fovDeg, float, 40.0f, SetProjectionDirty, _)

		MFA_VARIABLE2(farPlane, float, 1000.0f, SetProjectionDirty, _)

		MFA_VARIABLE2(nearPlane, float, 0.01f, SetProjectionDirty, _)

	    WindowExtendCallback _windowExtendCallback;
	    HasFocusCallback _windowHasFocusCallback;

		Transform _transform{};
		
		glm::mat4 _viewMat{};
		glm::mat4 _projMat{};
		glm::mat4 _viewProjMat{};

		bool _isProjectionDirty = true;
		bool _isViewDirty = true;
		bool _isViewProjectionDirty = true;
	};

}

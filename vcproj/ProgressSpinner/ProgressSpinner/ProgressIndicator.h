#ifndef _ProgressIndicator_H
#define _ProgressIndicator_H

#include "Windows.h"

namespace Gdiplus
{
	class PointF;
}

/** \class progress_spinner::ProgressIndicator
*	Progress spinner.
*/
class ProgressIndicator
{
public:
	/** \brief Constructor
	 *	\param parentWindow the handle of the parent window for the progress spinner
	 *	\param applicationInstance the handle of the module instance which used to create the parent window
	 *	\param circleColor the color of the circle in the progress spinner
	 *	\param backgroundColor the color of the background
	 *	\param numberOfPositions number of positions for the circle center, this means how many frames we have for one cycle
	 *	\param framesPerSecond frame rate of the animation
	 */
	ProgressIndicator(HWND parentWindow, HINSTANCE applicationInstance, COLORREF circleColor,
		COLORREF backgroundColor, int numberOfPositions, int framesPerSecond);

	/**	\brief Sets the position and side length of the square which holds the progress spinner
	 *	\param x the x position of the square in the parent window
	 *	\param y the y position of the square in the parent window
	 *	\param sideLength side length of the square
	 */
	void SetPosition(int x, int y, int sideLength);

	/**	\brief Starts the animation of the progress spinner
	 */
	bool Start();

	/**	\brief Destructor
	 */
	~ProgressIndicator();

private:
	/**	\brief Callback of the timer which is used to control the animation
	 *	\param lpParam the data passed to the callback
	 *	\param TimerOrWaitFired this is always true for timer callback
	 */
	static void CALLBACK TimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired);

	/**	\brief Callback function which is used to handle message sent to the progress spinner window
	 *	\param hWnd the handle to the window
	 *	\param uMsg the message sent to the window
	 *	\param wParam additional message info
	 *	\param lParam additional message info
	 */
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/**	\brief Handler for message WM_PAINT
	 *	\param hWnd the handle to the window
	 */
	void CustomPaint(HWND hWnd);

	/**	\brief Sends message to update the animation of the progress spinner
	 *	\param hWnd the handle to the window
	 */
	void UpdateProgress(HWND hWnd);

	// how many circles we have
	static const int kNumberOfCircles = 8;

	// Handle of the progress spinner window
	HWND m_progressIndicator;

	// Color of the circle
	COLORREF m_circleColor;

	// Color of the background
	COLORREF m_backgroundColor;

	// Positions of the circle center
	Gdiplus::PointF* m_positions;

	// List of radii of each circle
	float m_radii[kNumberOfCircles];

	// Indicates whether the animation is started
	bool m_started;

	// Index in the m_positions list which is the position of the biggest circle
	int m_biggestCirlePositionIndex;

	// Number of positions for the circle center, this means how many frames we have for one cycle
	int m_numberOfPositions;

	// Frame rate of the animation
	int m_framesPerSecond;

	// Handle of the timer queue
	HANDLE m_timerQueue;

	// Handle of the timer
	HANDLE m_timer;

	// Token of the gdiplus library
	ULONG_PTR m_gdiplusToken;
};
#endif
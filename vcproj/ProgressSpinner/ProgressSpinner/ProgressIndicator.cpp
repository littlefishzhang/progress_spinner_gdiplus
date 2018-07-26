#include "stdafx.h"
#include "ProgressIndicator.h"
#include "math.h"
#include "objidl.h"
#include "gdiplus.h"
#pragma comment (lib, "Gdiplus.lib")

#define WM_USER_UPDATEPROGRESS (WM_USER + 100)

namespace
{
	const double kNumberOfDegreesInCircle = 360;
	const double kNumberOfDegreesInHalfCircle = kNumberOfDegreesInCircle / 2;
	const double kPi = 3.1415926535897931;

	const LPCTSTR kLoadingScreenProgressIndicatorClassName = L"ProgressIndicator";

	// The circle centers of all the circles in the progress spinner is on the same big circle,
	// this method gets the center of this circle
	POINT GetCenterPoint(int width, int height)
	{
		POINT point;
		point.x = width / 2;
		point.y = height / 2;

		return point;
	}

	// Gets the coordinate of the circle centers in the window of the progress spinner
	Gdiplus::PointF GetCoordinate(const POINT& center, float radius, float angle)
	{
		double angleInRadian = kPi * angle / kNumberOfDegreesInHalfCircle;
		Gdiplus::PointF point;
		point.X = (float)center.x + radius * cos(angleInRadian);
		point.Y = (float)center.y + radius * sin(angleInRadian);
		return point;
	}

	// Gets the radius of each circle
	float GetCircleRadius(float largestCircleRadius, int index)
	{
		float currentRadius = largestCircleRadius - (float)index * 0.5;
		// if radius of the circle is 1, the circle will look like star,
		// so we set the minimum radius to 1.5 which looks better
		return currentRadius < 1.5 ? 1.5 : currentRadius;
	}
}

ProgressIndicator::ProgressIndicator(HWND parentWindow, HINSTANCE applicationInstance,
	COLORREF circleColor, COLORREF backgroundColor, int numberOfPositions, int framesPerSecond)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;

	// Initialize GDI+.
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	WNDCLASSEX progressIndicatorClass;

	progressIndicatorClass.cbSize = sizeof(WNDCLASSEX);
	progressIndicatorClass.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	progressIndicatorClass.lpfnWndProc = WindowProc;
	progressIndicatorClass.cbClsExtra = 0;
	progressIndicatorClass.cbWndExtra = sizeof(ProgressIndicator*);
	progressIndicatorClass.hInstance = applicationInstance;
	progressIndicatorClass.hIcon = NULL;
	progressIndicatorClass.hCursor = NULL;
	// set NULL to background brush so that we don't repaint the background when erasing the background
	// we paint the background when handling WM_PAINT message, this could avoid flickering
	progressIndicatorClass.hbrBackground = NULL;
	progressIndicatorClass.lpszMenuName = NULL;
	progressIndicatorClass.lpszClassName = kLoadingScreenProgressIndicatorClassName;
	progressIndicatorClass.hIconSm = NULL;

	if (!RegisterClassEx(&progressIndicatorClass))
		return;

	m_framesPerSecond = framesPerSecond;
	m_numberOfPositions = numberOfPositions;

	m_progressIndicator = CreateWindowEx(NULL, kLoadingScreenProgressIndicatorClassName, NULL, WS_CHILD | WS_VISIBLE, 1, 1, 1, 1, parentWindow, NULL, applicationInstance, this);

	m_circleColor = circleColor;
	m_backgroundColor = backgroundColor;
	m_started = false;

	m_positions = NULL;
}

void ProgressIndicator::SetPosition(int x, int y, int sideLength)
{
	POINT center = GetCenterPoint(sideLength, sideLength);

	float largestCircleRadius = (float)sideLength / 10;

	// The circle centers of all the circles in the progress spinner is on the same big circle,
	// this is the radius of this big circle
	float bigRadius = (float)sideLength / 2 - largestCircleRadius - 1.0;

	m_positions = new Gdiplus::PointF[m_numberOfPositions];
	for (int i = 0; i < m_numberOfPositions; ++i)
	{
		m_positions[i] = GetCoordinate(center, bigRadius, (float)i * 360 / m_numberOfPositions);
	}

	for (int i = 0; i < kNumberOfCircles; ++i)
	{
		m_radii[i] = GetCircleRadius(largestCircleRadius, i);
	}

	SetWindowPos(m_progressIndicator, NULL, x, y, sideLength, sideLength, SWP_NOZORDER | SWP_NOACTIVATE);
}

LRESULT CALLBACK ProgressIndicator::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		CREATESTRUCT* createInfo = reinterpret_cast<CREATESTRUCT*>(lParam);
		SetWindowLongPtr(hWnd, 0, (LONG_PTR)createInfo->lpCreateParams);
		return S_OK;
	}
	case WM_PAINT:
	{
		ProgressIndicator* progressIndicator = (ProgressIndicator*)GetWindowLongPtr(hWnd, 0);
		if (progressIndicator != NULL)
		{
			progressIndicator->CustomPaint(hWnd);
			return S_OK;
		}
		else
		{
			return S_OK;
		}
	}
	case WM_USER_UPDATEPROGRESS:
	{
		ProgressIndicator* progressIndicator = (ProgressIndicator*)GetWindowLongPtr(hWnd, 0);
		if (progressIndicator != NULL)
		{

			progressIndicator->UpdateProgress(hWnd);
			return S_OK;
		}
		else
		{
			return S_OK;
		}
	}
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	}
}

void ProgressIndicator::CustomPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	RECT rect;
	GetClientRect(hWnd, &rect);
	HDC hdc = BeginPaint(hWnd, &ps);

	Gdiplus::Graphics graphics(hdc);
	Gdiplus::Bitmap bitmap(rect.right - rect.left, rect.bottom - rect.top);
	Gdiplus::Graphics* graph = Gdiplus::Graphics::FromImage(&bitmap);
	graph->SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);
	Gdiplus::SolidBrush backgroundBrush(Gdiplus::Color(255, GetRValue(m_backgroundColor), GetGValue(m_backgroundColor), GetBValue(m_backgroundColor)));
	graph->FillRectangle(&backgroundBrush, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);

	for (int i = 0; i < kNumberOfCircles; ++i)
	{
		int cur = m_biggestCirlePositionIndex;

		int currentPointIndex = (cur + i *  m_numberOfPositions / kNumberOfCircles) % m_numberOfPositions;
		Gdiplus::PointF& currentPoint = m_positions[currentPointIndex];
		float curRadius = m_radii[i];

		Gdiplus::SolidBrush solidBrush(Gdiplus::Color(255, GetRValue(m_circleColor), GetGValue(m_circleColor), GetBValue(m_circleColor)));

		graph->FillEllipse(&solidBrush, currentPoint.X - curRadius, currentPoint.Y - curRadius, 2 * curRadius, 2 * curRadius);
	}

	graphics.DrawImage(&bitmap, 0, 0, rect.right - rect.left, rect.bottom - rect.top);
	EndPaint(hWnd, &ps);
}

bool ProgressIndicator::Start()
{
	// Create timer which runs in a seperate thread,
	// so that the callback of the timer could have a higher priority than WM_TIMER
	m_timerQueue = CreateTimerQueue();
	if (m_timerQueue == NULL)
	{
		return false;
	}
	BOOL result = CreateTimerQueueTimer(&m_timer, m_timerQueue, (WAITORTIMERCALLBACK)TimerCallback, &m_progressIndicator, 1000 / m_framesPerSecond, 1000 / m_framesPerSecond, 0);

	if (result == 0)
	{
		return false;
	}

	m_started = true;
	m_biggestCirlePositionIndex = 0;
	return true;
}

void ProgressIndicator::UpdateProgress(HWND hWnd)
{
	if (m_started)
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		m_biggestCirlePositionIndex = (m_biggestCirlePositionIndex + 1) % m_numberOfPositions;
		// Don't erase the background, we paint the background when handling WM_PAINT message,
		// this could avoid flickering
		InvalidateRect(hWnd, &rect, FALSE);
	}
}

void CALLBACK ProgressIndicator::TimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	SendMessage(*((HWND*)lpParam), WM_USER_UPDATEPROGRESS, 0, 0);
}

ProgressIndicator::~ProgressIndicator()
{
	if (m_positions != NULL)
	{
		delete[] m_positions;
	}

	DeleteTimerQueueTimer(m_timerQueue, m_timer, NULL);
	DeleteTimerQueue(m_timerQueue);

	Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

#include "pr.hpp"
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include <iostream>
#include <memory>

#include "lwHoudiniLoader.hpp"

int main()
{
	using namespace pr;

	SetDataDir( JoinPath( ExecutableDir(), "..", "data" ) );

	Config config;
	config.ScreenWidth = 1920;
	config.ScreenHeight = 1080;
	config.SwapInterval = 1;
	Initialize( config );

	Camera3D camera;
	camera.origin = {4, 4, 4};
	camera.lookat = {0, 0, 0};
	camera.zUp = false;

	BinaryLoader loader;
	loader.load( "out/box.json" );
	loader.push_back( '\0' );

	Stopwatch sw;
	rapidjson::Document d;
	d.ParseInsitu( (char*)loader.data() );
	PR_ASSERT( d.HasParseError() == false );

	auto lwhPolygon = lwh::load(d);

	//Polygon polygon;
	//polygon.xform = lwhPolygon.polygon->xform;
	//polygon.indices = std::move(lwhPolygon.polygon->indices);
	//polygon.P = std::move(lwhPolygon.polygon->P);

	printf( "load: %f ms", sw.elapsed() * 1000.0f );

	double e = GetElapsedTime();

	while ( pr::NextFrame() == false )
	{
		if ( IsImGuiUsingMouse() == false )
		{
			UpdateCameraBlenderLike( &camera );
		}

		ClearBackground( 0.1f, 0.1f, 0.1f, 1 );

		BeginCamera( camera );

		PushGraphicState();

		DrawGrid( GridAxis::XZ, 1.0f, 10, {128, 128, 128} );
		DrawXYZAxis( 1.0f );

		BeginCameraWithObjectTransform( camera, lwhPolygon.polygon->xform );
		PrimBegin( PrimitiveMode::Lines );
		for ( int i = 0; i < lwhPolygon.polygon->indices.size(); i += 3 )
		{
			int a = lwhPolygon.polygon->indices[i];
			int b = lwhPolygon.polygon->indices[i + 1];
			int c = lwhPolygon.polygon->indices[i + 2];
			glm::u8vec3 color = {
				255, 
				255, 
				255};
			PrimVertex( lwhPolygon.polygon->P[a], color );
			PrimVertex( lwhPolygon.polygon->P[b], color );
			PrimVertex( lwhPolygon.polygon->P[b], color );
			PrimVertex( lwhPolygon.polygon->P[c], color );
			PrimVertex( lwhPolygon.polygon->P[c], color );
			PrimVertex( lwhPolygon.polygon->P[a], color );
		}
		PrimEnd();

		for (int i = 0; i < lwhPolygon.polygon->P.size(); i++)
		{
			glm::u8vec3 color = glm::u8vec3( lwhPolygon.polygon->pointsVectorAttrib["Cd"][i] * 255.0f + glm::vec3(0.5f) );
			DrawSphere(lwhPolygon.polygon->P[i], 0.05f, color);
		}
		for (int i = 0; i < lwhPolygon.polygon->indices.size(); i++)
		{
			int index = lwhPolygon.polygon->indices[i];
			auto p = lwhPolygon.polygon->P[index];
			auto n = lwhPolygon.polygon->verticesVectorAttrib["N"][i];
			DrawArrow(p, p + n * 0.1f, 0.002f, { 255, 255, 0 });
		}
		EndCamera();

		PopGraphicState();
		EndCamera();

		BeginImGui();

		ImGui::SetNextWindowSize( {500, 800}, ImGuiCond_Once );
		ImGui::Begin( "Panel" );
		ImGui::Text( "fps = %f", GetFrameRate() );

		ImGui::End();

		EndImGui();
	}

	pr::CleanUp();
}

#pragma once
#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/Shape.h>
#include "actor/terrain-structs.hpp"
#include "physics/jolt-conversions.hpp"

#include "Jolt/Physics/Collision/CastConvexVsTriangles.h"
#include "Jolt/Physics/Collision/CollideConvexVsTriangles.h"
#include "Jolt/Physics/Collision/CollisionDispatch.h"
#include "Jolt/Geometry/RayTriangle.h"


class TerrainShapeSettings : public JPH::ShapeSettings {

    public:
    TerrainShapeSettings(MeshData<TerrainVertex>& terrain,float size) : terrain(terrain), size(size) {
        
    }

    JPH::Shape::ShapeResult Create() const;

    

    MeshData<TerrainVertex>& terrain;
    float size;

};


#define TERRAIN_SHAPE_TYPE JPH::EShapeType::User1
#define TERRAIN_SHAPE_SUB_TYPE JPH::EShapeSubType::User1

class TerrainShape : public JPH::Shape {

    
    

    MeshData<TerrainVertex> meshData;
    JPH::AABox localBounds;
    
    JPH::AABox GetLocalBounds() const override {

        return localBounds;

    }
    
    public:
    TerrainShape() : Shape(TERRAIN_SHAPE_TYPE,TERRAIN_SHAPE_SUB_TYPE) {};
    TerrainShape(const TerrainShapeSettings &inSettings, ShapeResult &outResult) : Shape(TERRAIN_SHAPE_TYPE,TERRAIN_SHAPE_SUB_TYPE), meshData(inSettings.terrain) {
        localBounds.mMin = JPH::Vec3(0.0f,0.0f,0.0f);
        localBounds.mMax = Physics::toJoltVec(glm::vec3(inSettings.size));
    }

    struct Triangle {
        JPH::Vec3 a;
        JPH::Vec3 b;
        JPH::Vec3 c;
    };

    Triangle getTriangle(size_t index) const {

        Triangle tri;
        size_t indexA = meshData.indices[index];
        size_t indexB = meshData.indices[index+1];
        size_t indexC = meshData.indices[index+2];
        tri.a = Physics::toJoltVec(meshData.vertices[indexA].pos);
        tri.b = Physics::toJoltVec(meshData.vertices[indexB].pos);
        tri.c = Physics::toJoltVec(meshData.vertices[indexC].pos);
        return tri;
    }

    void UpdateMesh(const MeshData<TerrainVertex>& newMeshData) {
        meshData = newMeshData;
    }

    inline unsigned int GetSubShapeIDBitsRecursive() const override { return 0; } // no sub shapes I think?
    inline float GetInnerRadius() const override { return 0.0f; }
    inline JPH::MassProperties GetMassProperties() const override { return JPH::MassProperties();}
    inline const JPH::PhysicsMaterial* GetMaterial(const JPH::SubShapeID &inSubShapeID) const override { return JPH::PhysicsMaterial::sDefault; } // for now
    inline void GetTrianglesStart(JPH::Shape::GetTrianglesContext &ioContext, const JPH::AABox &inBox, JPH::Vec3Arg inPositionCOM, JPH::QuatArg inRotation, JPH::Vec3Arg inScale) const override { }
    inline int	GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, JPH::Float3 *outTriangleVertices, const JPH::PhysicsMaterial **outMaterials = nullptr) const override {
        return 0;
    }
    inline JPH::Vec3 GetSurfaceNormal(const JPH::SubShapeID &inSubShapeID, JPH::Vec3Arg inLocalSurfacePosition) const override { 
        
        JPH::SubShapeID remainder;
        JPH::uint32 idx = inSubShapeID.PopID(32,remainder);

        size_t triIndex = idx * 3;

        if(triIndex+2 < meshData.indices.size()) {
            
            auto triangle = getTriangle(triIndex);

            return (triangle.c - triangle.b).Cross(triangle.a  - triangle.c).Normalized();
        }

        std::cout << triIndex << " is the id. ew" << std::endl;

        return JPH::Vec3(0.0f,1.0f,0.0f);

    }

    inline virtual bool CastRay(const JPH::RayCast &inRay, const JPH::SubShapeIDCreator &inSubShapeIDCreator, JPH::RayCastResult &ioHit) const override {
        
        bool foundHit = false;
        // i know this isn't the best way of doing this, maybe a visitor would be good?
        for (size_t i = 0; i+2 < meshData.indices.size(); i += 3)
        {
            size_t indexA = meshData.indices[i];
            size_t indexB = meshData.indices[i+1];
            size_t indexC = meshData.indices[i+2];
            //std::cout << indexA << " " << shape2->terrain->vertices.size() << " " << &shape2->terrain->vertices << std::endl;
            JPH::Vec3 a = Physics::toJoltVec(meshData.vertices[indexA].pos);
            JPH::Vec3 b = Physics::toJoltVec(meshData.vertices[indexB].pos);
            JPH::Vec3 c = Physics::toJoltVec(meshData.vertices[indexC].pos);
            int id = i/3;
            float result = JPH::RayTriangle(inRay.mOrigin,inRay.mDirection,a,b,c);
            if(result < ioHit.mFraction) {
                
                ioHit.mFraction = result;
                int id = i/3;
                auto triangle_id = inSubShapeIDCreator.PushID(id,32);
		        ioHit.mSubShapeID2 = triangle_id.GetID();
                foundHit = true;
            }
            
        }
        return foundHit;
    }

	/// Cast a ray against this shape. Allows returning multiple hits through ioCollector. Note that this version is more flexible but also slightly slower than the CastRay function that returns only a single hit.
	/// If you want the surface normal of the hit use GetSurfaceNormal(collected sub shape ID, inRay.GetPointOnRay(collected faction)).
	inline void CastRay(const JPH::RayCast &inRay, const JPH::RayCastSettings &inRayCastSettings, const JPH::SubShapeIDCreator &inSubShapeIDCreator, JPH::CastRayCollector &ioCollector, const JPH::ShapeFilter &inShapeFilter = { }) const override { 


        for (size_t i = 0; i+2 < meshData.indices.size(); i += 3)
        {
            size_t indexA = meshData.indices[i];
            size_t indexB = meshData.indices[i+1];
            size_t indexC = meshData.indices[i+2];
            //std::cout << indexA << " " << shape2->terrain->vertices.size() << " " << &shape2->terrain->vertices << std::endl;
            JPH::Vec3 a = Physics::toJoltVec(meshData.vertices[indexA].pos);
            JPH::Vec3 b = Physics::toJoltVec(meshData.vertices[indexB].pos);
            JPH::Vec3 c = Physics::toJoltVec(meshData.vertices[indexC].pos);
            int id = i/3;
            float result = JPH::RayTriangle(inRay.mOrigin,inRay.mDirection,a,b,c);
            if(result != FLT_MAX) {
                JPH::RayCastResult hit;
                hit.mFraction = result;
                int id = i/3;
                auto triangle_id = inSubShapeIDCreator.PushID(id,32);
		        hit.mSubShapeID2 = triangle_id.GetID();
                ioCollector.AddHit(hit);
            }
            
        }

    }

    inline void GetSubmergedVolume(JPH::Mat44Arg inCenterOfMassTransform, JPH::Vec3Arg inScale, const JPH::Plane &inSurface, float &outTotalVolume, float &outSubmergedVolume, JPH::Vec3 &outCenterOfBuoyancy
    #ifdef JPH_DEBUG_RENDERER // Not using JPH_IF_DEBUG_RENDERER for Doxygen
		, JPH::RVec3Arg inBaseOffset
#endif
    ) const override { }

    inline void CollidePoint(JPH::Vec3Arg inPoint, const JPH::SubShapeIDCreator &inSubShapeIDCreator, JPH::CollidePointCollector &ioCollector, const JPH::ShapeFilter &inShapeFilter = { }) const override { }

    inline void CollideSoftBodyVertices(JPH::Mat44Arg inCenterOfMassTransform, JPH::Vec3Arg inScale, const JPH::CollideSoftBodyVertexIterator &inVertices, uint inNumVertices, int inCollidingShapeIndex) const override { }

    inline virtual JPH::Shape::Stats GetStats() const override { 
        return JPH::Shape::Stats(sizeof(this),meshData.vertices.size());
    }

    inline virtual float GetVolume() const override {
        return 0;
    }

    #ifdef JPH_DEBUG_RENDERER

        /// Draw the shape at a particular location with a particular color (debugging purposes)
        virtual void Draw(JPH::DebugRenderer *inRenderer, JPH::RMat44Arg inCenterOfMassTransform, JPH::Vec3Arg inScale, JPH::ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const override {}

    #endif // JPH_DEBUG_RENDERER

    inline static void sCollideConvexVsMesh(const Shape *inShape1, const Shape *inShape2, JPH::Vec3Arg inScale1, JPH::Vec3Arg inScale2, JPH::Mat44Arg inCenterOfMassTransform1, JPH::Mat44Arg inCenterOfMassTransform2, const JPH::SubShapeIDCreator &inSubShapeIDCreator1, const JPH::SubShapeIDCreator &inSubShapeIDCreator2, const JPH::CollideShapeSettings &inCollideShapeSettings, JPH::CollideShapeCollector &ioCollector, [[maybe_unused]] const JPH::ShapeFilter &inShapeFilter)
    {
        //JPH_PROFILE_FUNCTION();

        

        // Get the shapes
        assert(inShape1->GetType() == JPH::EShapeType::Convex);
        assert(inShape2->GetType() == TERRAIN_SHAPE_TYPE);

        const JPH::ConvexShape *shape1 = static_cast<const JPH::ConvexShape *>(inShape1);
	    const TerrainShape *shape2 = static_cast<const TerrainShape *>(inShape2);

        auto collide = JPH::CollideConvexVsTriangles(shape1,inScale1,inScale2,inCenterOfMassTransform1,inCenterOfMassTransform2,inSubShapeIDCreator1.GetID(), inCollideShapeSettings, ioCollector);

        auto& terrain = shape2->meshData;
        for (size_t i = 0; i+2 < terrain.indices.size(); i += 3)
        {
            size_t indexA = terrain.indices[i];
            size_t indexB = terrain.indices[i+1];
            size_t indexC = terrain.indices[i+2];
            //std::cout << indexA << " " << shape2->terrain->vertices.size() << " " << &shape2->terrain->vertices << std::endl;
            JPH::Vec3 a = Physics::toJoltVec(terrain.vertices[indexA].pos);
            JPH::Vec3 b = Physics::toJoltVec(terrain.vertices[indexB].pos);
            JPH::Vec3 c = Physics::toJoltVec(terrain.vertices[indexC].pos);


            int id = i/3;
            auto triangle_id = inSubShapeIDCreator2.PushID(id,32);
            collide.Collide(a,b,c,255,triangle_id.GetID());
        }
        
    }

    static void sRegister()
    {
        
        JPH::ShapeFunctions &f = JPH::ShapeFunctions::sGet(TERRAIN_SHAPE_SUB_TYPE);
        f.mConstruct = []() -> Shape * { return new TerrainShape; };
        f.mColor = JPH::Color::sRed;

        for (JPH::EShapeSubType s : JPH::sConvexSubShapeTypes)
	    {
            JPH::CollisionDispatch::sRegisterCollideShape(s, TERRAIN_SHAPE_SUB_TYPE, sCollideConvexVsMesh);
        }
        // Specialized collision functions
        
        //JPH::CollisionDispatch::sRegisterCastShape(EShapeSubType::Sphere, EShapeSubType::Mesh, sCastSphereVsMesh);
    }

};


JPH::Shape::ShapeResult TerrainShapeSettings::Create() const {
    if (mCachedResult.IsEmpty()) {
		    JPH::Ref<JPH::Shape> shape = new TerrainShape(*this, mCachedResult);
        }
	    return mCachedResult;   

}

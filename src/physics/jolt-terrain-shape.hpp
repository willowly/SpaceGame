#pragma once
#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/Shape.h>
#include "actor/terrain-structs.hpp"
#include "physics/jolt-conversions.hpp"

#include "Jolt/Physics/Collision/CastConvexVsTriangles.h"
#include "Jolt/Physics/Collision/CollideConvexVsTriangles.h"
#include "Jolt/Physics/Collision/CollisionDispatch.h"
#include "Jolt/Geometry/RayTriangle.h"

#include <cmath> // for cube root
#include <math.h>


class TerrainShapeSettings : public JPH::ShapeSettings {

    public:
    TerrainShapeSettings(MeshData<TerrainVertex>& terrain,std::vector<VoxelData>& voxelData,float size) : terrain(terrain), voxelData(voxelData), size(size) {
        
    }

    JPH::Shape::ShapeResult Create() const;

    

    MeshData<TerrainVertex>& terrain;
    std::vector<VoxelData>& voxelData;
    float size;

};


#define TERRAIN_SHAPE_TYPE JPH::EShapeType::User1
#define TERRAIN_SHAPE_SUB_TYPE JPH::EShapeSubType::User1

class TerrainShape : public JPH::Shape {

    
    

    MeshData<TerrainVertex> meshData;
    std::vector<VoxelData> voxelData;
    JPH::AABox localBounds;
    int sizeInCells;
    float cellSize;
    
    JPH::AABox GetLocalBounds() const override {

        return localBounds;

    }
    
    public:
    TerrainShape() : Shape(TERRAIN_SHAPE_TYPE,TERRAIN_SHAPE_SUB_TYPE) {};
    TerrainShape(const TerrainShapeSettings &inSettings, ShapeResult &outResult) : Shape(TERRAIN_SHAPE_TYPE,TERRAIN_SHAPE_SUB_TYPE), meshData(inSettings.terrain), voxelData(inSettings.voxelData) {
        localBounds.mMin = JPH::Vec3(0.0f,0.0f,0.0f);
        localBounds.mMax = Physics::toJoltVec(glm::vec3(inSettings.size));
        sizeInCells = (int)std::cbrt((float)voxelData.size());
        cellSize = inSettings.size / sizeInCells;
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

    void UpdateMesh(const MeshData<TerrainVertex>& newMeshData,const std::vector<VoxelData>& newVoxelData) {
        meshData = newMeshData;
        voxelData = newVoxelData;
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

    inline void CollideSoftBodyVertices(JPH::Mat44Arg inCenterOfMassTransform, JPH::Vec3Arg inScale, const JPH::CollideSoftBodyVertexIterator &inVertices, unsigned int inNumVertices, int inCollidingShapeIndex) const override { }

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

    static int getPointIndex(int x,int y,int z,int size) {
        x = std::clamp(x,0,size-1);
        y = std::clamp(y,0,size-1);
        z = std::clamp(z,0,size-1);
        return x + y * size + z * size * size;
    }

    inline static void sCollideConvexVsMesh(const Shape *inShape1, const Shape *inShape2, JPH::Vec3Arg inScale1, JPH::Vec3Arg inScale2, JPH::Mat44Arg inCenterOfMassTransform1, JPH::Mat44Arg inCenterOfMassTransform2, const JPH::SubShapeIDCreator &inSubShapeIDCreator1, const JPH::SubShapeIDCreator &inSubShapeIDCreator2, const JPH::CollideShapeSettings &inCollideShapeSettings, JPH::CollideShapeCollector &ioCollector, [[maybe_unused]] const JPH::ShapeFilter &inShapeFilter)
    {
        //JPH_PROFILE_FUNCTION();

        

        // Get the shapes
        assert(inShape1->GetType() == JPH::EShapeType::Convex);
        assert(inShape2->GetType() == TERRAIN_SHAPE_TYPE);

        const JPH::ConvexShape *shape1 = static_cast<const JPH::ConvexShape *>(inShape1);
	    const TerrainShape *shape2 = static_cast<const TerrainShape *>(inShape2);

        auto collide = JPH::CollideConvexVsTriangles(shape1,inScale1,inScale2,inCenterOfMassTransform1,inCenterOfMassTransform2,inSubShapeIDCreator1.GetID(), inCollideShapeSettings, ioCollector);
            
        auto bounds1 = inShape1->GetWorldSpaceBounds(inCenterOfMassTransform1,inScale1);

        auto transformedBounds1 = bounds1.Transformed(inCenterOfMassTransform2.Inversed()); //Inverse transform into shape 2 local space

        auto cellSize = shape2->cellSize;
        transformedBounds1.mMin /= cellSize;
        transformedBounds1.mMax /= cellSize;

        auto& voxelData = shape2->voxelData;
        auto& meshData = shape2->meshData;

        //std::cout << "min: " <<  StringHelper::toString(Physics::toGlmVec(transformedBounds1.mMin)) << std::endl;

        for (int z = std::max(0,(int)floor(transformedBounds1.mMin.GetZ())); z <= std::min(shape2->sizeInCells-1,(int)ceil(transformedBounds1.mMax.GetZ())); z++)
        {
            for (int y = std::max(0,(int)floor(transformedBounds1.mMin.GetY())); y <= std::min(shape2->sizeInCells-1,(int)ceil(transformedBounds1.mMax.GetY())); y++)
            {
                for (int x = std::max(0,(int)floor(transformedBounds1.mMin.GetX())); x <= std::min(shape2->sizeInCells-1,(int)ceil(transformedBounds1.mMax.GetX())); x++)
                {

                    auto voxel = voxelData[getPointIndex(x,y,z,shape2->sizeInCells)];
                    for (int i = voxel.verticesStart;i < voxel.verticesEnd;i += 3)
                    {
                        
                        size_t indexA = meshData.indices[i];
                        size_t indexB = meshData.indices[i+1];
                        size_t indexC = meshData.indices[i+2];
                        //std::cout << indexA << " " << shape2->terrain->vertices.size() << " " << &shape2->terrain->vertices << std::endl;
                        JPH::Vec3 a = Physics::toJoltVec(meshData.vertices[indexA].pos);
                        JPH::Vec3 b = Physics::toJoltVec(meshData.vertices[indexB].pos);
                        JPH::Vec3 c = Physics::toJoltVec(meshData.vertices[indexC].pos);

                        // Debug::drawLine(meshData.vertices[indexA].pos,meshData.vertices[indexB].pos,Color::green,0.01f);
                        // Debug::drawLine(meshData.vertices[indexB].pos,meshData.vertices[indexC].pos,Color::green,0.01f);
                        // Debug::drawLine(meshData.vertices[indexC].pos,meshData.vertices[indexA].pos,Color::green,0.01f);


                        int id = i/3;
                        auto triangle_id = inSubShapeIDCreator2.PushID(id,32);
                        collide.Collide(a,b,c,255,triangle_id.GetID());
                            
                    } 
                    //Debug::drawCube((vec3(x,y,z)+vec3(0.5f))*cellSize + Physics::toGlmVec(inCenterOfMassTransform2.GetTranslation()),vec3(cellSize),glm::identity<quat>(),Color::green,0.03f);
                    //std::cout << "draw cube " << StringHelper::toString((vec3(x,y,z)+vec3(0.5f))) << std::endl;
                }

            }
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
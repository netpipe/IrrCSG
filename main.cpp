#include <irrlicht.h>
#include "driverChoice.h"
#include "CSG/primitives.h"
#include "CSG/CSG.h"

using namespace irr;

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

class CSampleSceneNode : public scene::ISceneNode {
	core::aabbox3d<f32> Box;
	video::S3DVertex *vertices;
	video::SMaterial Material;
	meshMesh mesh;
public:
	CSampleSceneNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id)
		: scene::ISceneNode(parent, mgr, id){
		Material.Wireframe = true;
		Material.Lighting = false;
		initializeMesh();
		//meshInitializeBox(&mesh, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5);

		//Vertices[0] = video::S3DVertex(0, 0, 10, 1, 1, 0, video::SColor(255, 0, 255, 255), 0, 1);
		//Vertices[1] = video::S3DVertex(10, 0, -10, 1, 1, 0, video::SColor(255, 255, 0, 255), 1, 1);
		//Vertices[2] = video::S3DVertex(0, 20, 0, 1, 1, 0, video::SColor(255, 255, 255, 0), 1, 0);
		//Vertices[3] = video::S3DVertex(-10, 0, -10, 0, 0, 1, video::SColor(255, 0, 255, 0), 0, 0);
		GLuint vertNum = mesh.vertNum;
		GLuint attrDim = mesh.attrDim;
		vertices = (video::S3DVertex*)malloc(mesh.vertNum * sizeof(video::S3DVertex));
		for(GLuint i=0; i<vertNum; i++){
			vertices[i].Pos.set(mesh.vert[i * attrDim  + 0], mesh.vert[i * attrDim  + 1], mesh.vert[i * attrDim  + 2]);
		}
		Box.reset(vertices[0].Pos);
		for(s32 i=1; i<vertNum; i++){
			Box.addInternalPoint(vertices[i].Pos);
		}
	}

	int initializeMesh(){
		meshMesh meshA;
		meshMesh meshB;
		meshMesh meshC;
		meshMesh meshD;
		if (meshInitializeBox(&meshB, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5) != 0 || meshInitializeCapsule(&meshA, 0.5, 2.0, 16, 32)
			|| 	meshInitializeBox(&meshC, -0.3, 0.3, -0.3, 0.3, -0.3, 0.3) || meshInitializeSphere(&meshD, 0.3, 16, 32)) {
		 	return 1;
		}
		GLdouble translation[3] = {0.5, 0.5, 0.5};
		GLdouble translation1[3] = {0.2, 0.2, 0.3};
		meshTranslate(&meshA, translation);
		meshTranslate(&meshD, translation1);

		// boolean operations: ((capsule intersect box) union another box) subtract sphere
		CSGIntersection(&meshA, &meshB, &mesh);
		meshDestroy(&meshA);
		meshDestroy(&meshB);
		CSGUnion(&mesh, &meshC, &mesh);
		meshDestroy(&meshC);
		CSGSubtraction(&mesh, &meshD, &mesh);
		meshDestroy(&meshD);
		return 0;
	}

	virtual void OnRegisterSceneNode(){
		if(IsVisible){
			SceneManager->registerNodeForRendering(this);
		}
		ISceneNode::OnRegisterSceneNode();
	}

	virtual void render(){
		u16 indices[] = {
			0, 2, 3,
			2, 1, 3,
			1, 0, 3,
			2, 0, 1
		};
		video::IVideoDriver* driver = SceneManager->getVideoDriver();
		driver->setMaterial(Material);
		driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
		driver->drawVertexPrimitiveList(&vertices[0], mesh.vertNum, mesh.tri, mesh.triNum, video::EVT_STANDARD, scene::EPT_TRIANGLES, video::EIT_32BIT);
	}

	virtual const core::aabbox3d<f32>& getBoundingBox() const {
		return Box;
	}

	virtual u32 getMaterialCount() const {
		return 1;
	}

	virtual video::SMaterial& getMaterial(u32 i){
		return Material;
	}
};

int main(){
	video::E_DRIVER_TYPE driverType = driverChoiceConsole();
	if(driverType == video::EDT_COUNT){
		return 1;
	}

	IrrlichtDevice* device = createDevice(driverType, core::dimension2d<u32>(640, 480), 16, false);
	if(device == 0){
		return 1;
	}
	device->setWindowCaption(L"Custom Scene Node - Isslicht Engine Demo");
	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();

	CSampleSceneNode* myNode = new CSampleSceneNode(smgr->getRootSceneNode(), smgr, 666);
	core::aabbox3d<f32> bBox = myNode->getBoundingBox();
	//bBox.
	smgr->addCameraSceneNodeMaya();
	scene::ICameraSceneNode* camera = device->getSceneManager()->getActiveCamera();
	if(camera){
		camera->setPosition(bBox.MinEdge);
		camera->setTarget(bBox.getCenter());
		camera->setFarValue(20000.0f);
		camera->setInputReceiverEnabled(true);

	}
	//scene::ISceneNodeAnimator* anim = smgr->createRotationAnimator(core::vector3df(0.8f, 0, 0.8f));
	//if(anim){
		//myNode->addAnimator(anim);
		//anim->drop();
		//anim = 0;
//	}
	myNode->drop();
	myNode = 0;

	u32 frames = 0;
	while(device->run()){
		driver->beginScene(true, true, video::SColor(0, 100, 100, 100));
		smgr->drawAll();
		driver->endScene();

		if(++frames == 100){
			core::stringw str =L"Irrlicht Engine [";
			str += driver->getName();
			str += L"] FPS: ";

			str += (s32)driver->getFPS();
			device->setWindowCaption(str.c_str());
			frames = 0;
		}
	}
	device->drop();
	return 0;
}

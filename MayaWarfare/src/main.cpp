#include "main.h"

// Declare our game instance
main game;

main::main()
    : _scene(NULL), _wireframe(false)
{
}

void main::initialize()
{
	// Load game scene from file
    _scene = Scene::load("res/demo.scene");
    // Get the box model and initialize its material parameter values and bindings
    Node* boxNode = _scene->findNode("box");
    Model* boxModel = dynamic_cast<Model*>(boxNode->getDrawable());
    Material* boxMaterial = boxModel->getMaterial();

    // Set the aspect ratio for the scene's camera to match the current resolution

    _scene->getActiveCamera()->setAspectRatio(getAspectRatio());
	_scene->getActiveCamera()->getNode()->setId("perspShape");
}

void main::finalize()
{
    SAFE_RELEASE(_scene);
}

void main::update(float elapsedTime)
{

	MessageType type = mayaData.read();

	while(type)
	{
		if (type == MessageType::mNewMesh)
		{
			Node* boxNode = _scene->findNode("box");
			Model* boxModel = dynamic_cast<Model*>(boxNode->getDrawable());
			Material* boxMaterial = boxModel->getMaterial();

			VertexLayout* verteciesData = nullptr;
			UINT numVertecies = 0;
			UINT* index = nullptr;
			UINT numIndex = 0;
			char* name = nullptr;
			mayaData.getNewMesh(name, verteciesData, numVertecies, index, numIndex);

			Node* triNode = Node::create(name);
			VertexFormat::Element element(VertexFormat::POSITION, 3);
			const VertexFormat vertFormat(&element, 1);

			nodeNames.push_back(name);
			//std::string tmp(name);
			meshVertecies[name] = verteciesData;

			Mesh* triMesh = Mesh::createMesh(vertFormat, numVertecies, true);
			triMesh->setVertexData((float*)verteciesData, 0, numVertecies);

			MeshPart* meshPart = triMesh->addPart(Mesh::PrimitiveType::TRIANGLES, Mesh::IndexFormat::INDEX32, numIndex, true);
			meshPart->setIndexData(index, 0, numIndex);


			//-------------TEST!----------------
			//Mesh* triMesh = Mesh::createMesh(vertFormat, 4, false);
			//float vertData[]{
			//	1.0f, 0.0f, 0.0f,
			//	1.0f, 1.0f, 0.0f,
			//	0.0f, 0.0f, 0.0f,
			//	0.0f, 1.0f, 0.0f };
			//int indexData[]{ 0, 1, 2, 1, 2, 3 };
			//triMesh->setVertexData(vertData);
			//MeshPart* test = triMesh->addPart(Mesh::TRIANGLES, Mesh::IndexFormat::INDEX32, 6, false);
			//test->setIndexData(indexData, 0, 6);
			//-------------END TEST!------------
			
			
			Model* triModel = Model::create(triMesh);
			triModel->setMaterial(boxMaterial);
			triNode->setDrawable(triModel);
			triNode->setEnabled(true);
			Node* copy = triNode->clone();
			//_scene->addNode(triNode);
			//Node* copy = _scene->findNode(name)->clone();
			//copy->setId(&names[(nodeNames.size() - 1)*2]);
			//nodeNames.push_back(&names[(nodeNames.size() - 1) * 2]);
			_scene->addNode(copy);
			
			triModel->release();
			triMesh->release();
			triNode->release();
		}
		else if (type == MessageType::mVertexChange)
		{
			VertexLayout* updatedVerteciesData = nullptr;
			UINT numVerteciesChanged = 0;
			UINT* index = nullptr;
			char* name = nullptr;
			mayaData.getVertexChanged(name, updatedVerteciesData, index, numVerteciesChanged);

			//Node* nodeChanged = _scene->findNode(name);
			VertexLayout* vertexData = meshVertecies[name];

			for (UINT i = 0; i < numVerteciesChanged; i++)
				vertexData[index[i]] = updatedVerteciesData[i];

			Model* mesh = dynamic_cast<Model*>(_scene->findNode(name)->getDrawable());
			mesh->getMesh()->setVertexData((float*)vertexData);
		}
		else if (type == MessageType::mTransform)
		{
			char* name;
			float* translations = nullptr;
			float* scale = nullptr;
			float* rotation = nullptr;

			mayaData.getNewTransform(name, translations, scale, rotation);

			Node* node = _scene->findNode(name);

			Quaternion newRot(rotation);

			Vector3 newTrans(translations[0], translations[1], translations[2]);

			Vector3 newScale(scale);
			node->set(newScale, newRot, newTrans);
		}

		else if (type == MessageType::mCamera)
		{
			float* mat1 = nullptr;
			float* mat2 = nullptr;
			float* mat3 = nullptr;
			float* mat4 = nullptr;

			mayaData.getNewCamera(mat1, mat2, mat3, mat4);

			Matrix projectionMatrix(mat1[0], mat1[1], mat1[2], mat1[3],
									mat2[0], mat2[1], mat2[2], mat2[3],
									mat3[0], mat3[1], mat3[2], mat3[3],
									mat4[0], mat4[1], mat4[2], mat4[3]);
			
			projectionMatrix.transpose();
			_scene->getActiveCamera()->setProjectionMatrix(projectionMatrix);
			
		
		}

		type = mayaData.read();
	}
	// Rotate model
	_scene->findNode("box")->translateX(0.01f);
	_scene->findNode("box")->rotateX(0.01f);

	for (std::vector<char*>::iterator it = nodeNames.begin(); it != nodeNames.end(); ++it)
	{
		//_scene->findNode(*it)->rotateX(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));
		//_scene->findNode(*it)->translateX(-0.001f * i);
		//i *= -1;
	}
}

void main::render(float elapsedTime)
{
    // Clear the color and depth buffers
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);

    // Visit all the nodes in the scene for drawing
    _scene->visit(this, &main::drawScene);
}

bool main::drawScene(Node* node)
{
    // If the node visited contains a drawable object, draw it
    Drawable* drawable = node->getDrawable(); 
	if (drawable)
		drawable->draw(_wireframe);

    return true;
}

void main::keyEvent(Keyboard::KeyEvent evt, int key)
{
    if (evt == Keyboard::KEY_PRESS)
    {
        switch (key)
        {
        case Keyboard::KEY_ESCAPE:
            exit();
            break;
        }
    }
}

void main::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    switch (evt)
    {
    case Touch::TOUCH_PRESS:
        _wireframe = !_wireframe;
        break;
    case Touch::TOUCH_RELEASE:
        break;
    case Touch::TOUCH_MOVE:
        break;
    };
}

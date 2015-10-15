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
	RenderState::StateBlock* block = RenderState::StateBlock::create();
	//block->setDepthFunction(RenderState::DepthFunction::DEPTH_LESS);
	//block->setFrontFace(RenderState::FrontFace::FRONT_FACE_CW);
	//block->setDepthTest(true);
	//block->setCullFace(true);
	//block->setDepthWrite(true);
	//boxMaterial->setStateBlock(block);
    // Set the aspect ratio for the scene's camera to match the current resolution

    _scene->getActiveCamera()->setAspectRatio(getAspectRatio());
	_scene->getActiveCamera()->getNode()->setId("perspShape");
	perspCam = _scene->getActiveCamera();

	Node* tmpNode = Node::create("topShape");
	orthoCam = Camera::createOrthographic(0, 0, _scene->getActiveCamera()->getNearPlane(), _scene->getActiveCamera()->getAspectRatio(),_scene->getActiveCamera()->getFarPlane());
	tmpNode->setCamera(orthoCam);
	_scene->addNode(tmpNode);
	//_scene->setActiveCamera(orthoCam);
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
			VertexFormat::Element elements[] = {
				VertexFormat::Element(VertexFormat::POSITION, 3),
				VertexFormat::Element(VertexFormat::NORMAL, 3) 
			};
			const VertexFormat vertFormat(elements, ARRAYSIZE(elements));

			nodeNames.push_back(name);
			
			meshVertecies[name] = verteciesData;

			Mesh* triMesh = Mesh::createMesh(vertFormat, numVertecies, true);
			triMesh->setVertexData((float*)verteciesData, 0, numVertecies);

			MeshPart* meshPart = triMesh->addPart(Mesh::PrimitiveType::TRIANGLES, Mesh::IndexFormat::INDEX32, numIndex, true);
			meshPart->setIndexData(index, 0, numIndex);

			//----- MATERIAL TEST--------------
			char* texturePath;
			Effect* effect = Effect::createFromFile("res/shaders/colored.vert", "res/shaders/colored.frag", "POINT_LIGHT_COUNT 1");
			Material* material = Material::create(effect);
			RenderState::StateBlock* block = RenderState::StateBlock::create();
			block->setCullFace(true);
			block->setDepthTest(true);
			material->setStateBlock(block);
			//Texture* tex = Texture::create(texturePath);
			Node* lightNode = Node::create("myLight");
			Light* myLight = Light::createPoint(Vector3(0.8f, 0, 0), 20);
			lightNode->setLight(myLight);

			lightNode->setTranslation(0.0f, 10.0f, 0.0f);
			
			//Bindings for vertex-shader

			material->setParameterAutoBinding("u_worldViewProjectionMatrix", RenderState::AutoBinding::WORLD_VIEW_PROJECTION_MATRIX);
			material->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", RenderState::AutoBinding::INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX);
			material->getParameter("u_ambientColor")->setValue(Vector3(0.1f, 0.1f, 0.1f));
			material->getParameter("u_pointLightColor[0]")->setValue(lightNode->getLight()->getColor());//->bindValue(lightNode->getLight(), &Light::getColor);
			material->getParameter("u_pointLightRange[0]")->setValue(myLight->getRange());//->bindValue(lightNode, &Node::getForwardVectorView);
			material->getParameter("u_pointLightPosition[0]")->bindValue(lightNode, &Node::getTranslationView);
			material->getParameter("u_pointLightRangeInverse[0]")->setValue(lightNode->getLight()->getRangeInverse());
			//Bindings for fragment shader
			//material->setParameterAutoBinding("u_ambientColor", RenderState::AutoBinding::SCENE_AMBIENT_COLOR);
			material->getParameter("u_diffuseColor")->setValue(Vector4(myLight->getColor().x, myLight->getColor().y, myLight->getColor().z, 0));

			//----- MATERIAL TEST END---------
			
			Model* triModel = Model::create(triMesh);
			triModel->setMaterial(material);
			triNode->setDrawable(triModel);
			triNode->setEnabled(true);

			lightNode->setDrawable(triModel);
			_scene->addNode(lightNode);

			Node* copy = triNode->clone();
			_scene->addNode(copy);
			
			triModel->release();
			triMesh->release();
			triNode->release();
			delete[] index;
		}
		else if (type == MessageType::mVertexChange)
		{
			VertexLayout* updatedVerteciesData = nullptr;
			UINT numVerteciesChanged = 0;
			UINT* index = nullptr;
			char* name = nullptr;
			mayaData.getVertexChanged(name, updatedVerteciesData, index, numVerteciesChanged);

			//Node* nodeChanged = _scene->findNode(name);
			VertexLayout* vertexData = meshVertecies.find(name)->second;

			//for (UINT i = 0; i < numVerteciesChanged; i++)
			//	vertexData[index[i]] = updatedVerteciesData[i];

			Model* mesh = dynamic_cast<Model*>(_scene->findNode(name)->getDrawable());
			mesh->getMesh()->setVertexData((float*)updatedVerteciesData);
			delete[] index;
			delete[] updatedVerteciesData;
			delete[] name;
		}
		else if (type == MessageType::mMaterial)
		{

			//material->getParameter("u_directionalLightColor[0]")->bindValue()
			
		}
		else if (type == MessageType::mTransform)
		{
			char* name;
			float translations[3];
			float scale[3];
			float rotation[4];

			mayaData.getNewTransform(name, translations, scale, rotation);

			Node* node = _scene->findNode(name);
			if (node)
			{
				Quaternion newRot(rotation);

				Vector3 newTrans(translations[0], translations[1], translations[2]);

				Vector3 newScale(scale);
				node->set(newScale, newRot, newTrans);
			}
			delete[] name;
		}
		else if (type == MessageType::mCamera)
		{
			char* name;
			float camMatrix[4][4];
			bool isOrtho = true;

			mayaData.getNewCamera(name, camMatrix, &isOrtho);
			
			Matrix projectionMatrix(camMatrix[0][0], camMatrix[1][0], camMatrix[2][0], camMatrix[3][0],
									camMatrix[0][1], camMatrix[1][1], camMatrix[2][1], camMatrix[3][1],
									camMatrix[0][2], camMatrix[1][2], camMatrix[2][2], camMatrix[3][2],
									camMatrix[0][3], camMatrix[1][3], camMatrix[2][3], camMatrix[3][3]);
			
			//projectionMatrix.transpose();

			if (isOrtho)
				_scene->setActiveCamera(orthoCam);
			else
				_scene->setActiveCamera(perspCam);

			_scene->getActiveCamera()->getNode()->setId(name);
			_scene->getActiveCamera()->setProjectionMatrix(projectionMatrix);
			
		}
		else if (type == MessageType::mNodeRemoved)
		{
			char* name = nullptr;

			mayaData.getRemoveNode(name);
			if (meshVertecies.find(name) != meshVertecies.end())
			{
				delete[] meshVertecies.find(name)->second;
				meshVertecies.erase(name);
				_scene->removeNode(_scene->findNode(name));
			}

			delete[] name;
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

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
    _scene = Scene::create();
    // Get the box model and initialize its material parameter values and bindings

	//block->setDepthFunction(RenderState::DepthFunction::DEPTH_LESS);
	//block->setFrontFace(RenderState::FrontFace::FRONT_FACE_CW);
	//block->setDepthTest(true);
	//block->setCullFace(true);
	//block->setDepthWrite(true);
	//boxMaterial->setStateBlock(block);
    // Set the aspect ratio for the scene's camera to match the current resolution
	
	Node* lightNode = Node::create("pointLightShape1");
	Light* light = Light::createPoint(Vector3(0.5f, 0.5f, 0.5f), 20);
	lightNode->setLight(light);
	lightNode->translate(Vector3(0, 0, 0));
	_scene->addNode(lightNode);
	lightNode->release();
	light->release();
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
			bool alreadyExisting = false;
			VertexLayout* verteciesData = nullptr;
			UINT numVertecies = 0;
			UINT* index = nullptr;
			UINT numIndex = 0;
			char* name = nullptr;
			mayaData.getNewMesh(name, verteciesData, numVertecies, index, numIndex);

			Node* triNode = _scene->findNode(name);
			Material* material = nullptr;
			if (triNode)
			{
				alreadyExisting = true;
				delete[] meshVertecies[name];
				material = static_cast<Model*>(triNode->getDrawable())->getMaterial();
				if (material)
					material->addRef();
				_scene->removeNode(triNode);
			}
			else
			{
				triNode = Node::create(name);
			}

			VertexFormat::Element elements[] = {
				VertexFormat::Element(VertexFormat::POSITION, 3),
				VertexFormat::Element(VertexFormat::NORMAL, 3),
				VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
			};
			const VertexFormat vertFormat(elements, ARRAYSIZE(elements));		

			meshVertecies[name] = verteciesData;

			Mesh* triMesh = Mesh::createMesh(vertFormat, numVertecies, true);
			triMesh->setVertexData((float*)verteciesData,0);

			MeshPart* meshPart = triMesh->addPart(Mesh::PrimitiveType::TRIANGLES, Mesh::IndexFormat::INDEX32, numIndex, true);
			meshPart->setIndexData(index, 0, numIndex);

			Model* triModel = Model::create(triMesh);

			if (material)
				triModel->setMaterial(material);
			triNode->setDrawable(triModel);

			Node* copy = triNode->clone();
			_scene->addNode(copy);

			triModel->release();
			triMesh->release();
			triNode->release();
			delete[] index;
			if (!alreadyExisting)
				nodeNames.push_back(name);
			else
				delete[] name;
		}
		else if (type == MessageType::mVertexChange)
		{
			VertexLayout* updatedVerteciesData = nullptr;
			UINT numVerteciesChanged = 0;
			UINT* index = nullptr;
			char* name = nullptr;
			mayaData.getVertexChanged(name, updatedVerteciesData, index, numVerteciesChanged);

			VertexLayout* vertexData = meshVertecies.find(name)->second;

			Model* mesh = dynamic_cast<Model*>(_scene->findNode(name)->getDrawable());
			mesh->getMesh()->setVertexData((float*)updatedVerteciesData);
			delete[] index;
			delete[] updatedVerteciesData;
			delete[] name;
		}
		else if (type == MessageType::mNewMaterial)
		{
			char* name;
			char* texturePath;
			float diffuseColor[3];
			float ambientColor[3];
			mayaData.getMaterial(name, texturePath, diffuseColor, ambientColor);

			bool modelsNeedNewMaterial = false;

			OurMaterial& ThisOurMaterial = ourMaterialMap[name];

			//Check if material used Texture before but uses Color now. Then materials need updates in models.
			if (ThisOurMaterial.diffuseTexFilePath.size() > 0 && !texturePath)
			{		
				ThisOurMaterial.diffuseTexFilePath.clear();
				ThisOurMaterial.diffuseTexFilePath = "";
				ThisOurMaterial.texture->release();
				ThisOurMaterial.texture = nullptr;
				modelsNeedNewMaterial = true;
			}
			//Check if material used Color befor but uses Texture now. Then materials need updates in models.
			else if (ThisOurMaterial.diffuseTexFilePath.size() == 0 && texturePath)
			{
				modelsNeedNewMaterial = true;
			}

			//If uses Texture set texture else set Color
			if (texturePath)
			{
				if (ThisOurMaterial.texture)
					ThisOurMaterial.texture->release();
				ThisOurMaterial.diffuseTexFilePath.clear();
				ThisOurMaterial.diffuseTexFilePath = texturePath;
				ThisOurMaterial.texture = Texture::Sampler::create(texturePath, false);
				ThisOurMaterial.texture->setFilterMode(Texture::LINEAR, Texture::LINEAR);
				ThisOurMaterial.texture->setWrapMode(Texture::CLAMP, Texture::CLAMP);
			}
			else
				ourMaterialMap[name].diffuseColor.set(diffuseColor[0], diffuseColor[1], diffuseColor[2], 1);

			ourMaterialMap[name].ambientColor.set(ambientColor);

			//If models need update on material, then update them.
			if (modelsNeedNewMaterial)
			{
				//loop tought all models;
				for (std::map<char*, std::string>::iterator it = NodeIDToMaterial.begin(); it != NodeIDToMaterial.end(); it++)
				{
					//if model uses this material, then create a new updated
					if (it->second.compare(name) == 0)
					{
						Material* newMaterial = nullptr;
						if (ThisOurMaterial.diffuseTexFilePath.size() > 0)
							newMaterial = Material::create("res/shaders/textured.vert", "res/shaders/textured.frag", "POINT_LIGHT_COUNT 1");
						else
							newMaterial = Material::create("res/shaders/colored.vert", "res/shaders/colored.frag", "POINT_LIGHT_COUNT 1");
						RenderState::StateBlock* block = RenderState::StateBlock::create();
						block->setCullFace(true);
						block->setDepthTest(true);
						newMaterial->setStateBlock(block);
						newMaterial->setParameterAutoBinding("u_worldViewMatrix", RenderState::AutoBinding::WORLD_VIEW_MATRIX);
						newMaterial->setParameterAutoBinding("u_worldViewProjectionMatrix", RenderState::AutoBinding::WORLD_VIEW_PROJECTION_MATRIX);
						newMaterial->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", RenderState::AutoBinding::INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX);

						Node* lightNode = _scene->findNode("pointLightShape1");
						newMaterial->getParameter("u_pointLightColor[0]")->bindValue(lightNode->getLight(), &Light::getColor);
						newMaterial->getParameter("u_pointLightRangeInverse[0]")->bindValue(lightNode->getLight(), &Light::getRangeInverse);
						newMaterial->getParameter("u_pointLightPosition[0]")->bindValue(lightNode, &Node::getTranslationView);

						if (texturePath)
							newMaterial->getParameter("u_diffuseTexture")->bindValue(&ourMaterialMap[name], &OurMaterial::getTexure);
						else
							newMaterial->getParameter("u_diffuseColor")->bindValue(&ourMaterialMap[name], &OurMaterial::getDiffuseColor);

						newMaterial->getParameter("u_ambientColor")->bindValue(&ourMaterialMap[name], &OurMaterial::getAmbientColor);
						
						Node* thisNode = _scene->findNode(it->first);
						Model* thisModel = static_cast<Model*>(thisNode->getDrawable());
						thisModel->setMaterial(newMaterial);
					}
				}
			}			
		}
		else if (type == MessageType::mMeshChangedMaterial)
		{
			char* meshName;
			char* materialName;

			mayaData.getMeshMaterialNames(meshName, materialName);

			char* nodeID = nullptr;
			std::string nodeNameString;
			//Find the id for the node with this name
			for (std::vector<char*>::iterator it = nodeNames.begin(); it != nodeNames.end(); it++)
			{
				nodeNameString = *it;
				if (nodeNameString.compare(meshName) == 0)
				{
					nodeID = *it;
					break;
				}
			}

			//If we found an ID, then change material
			if (nodeID)
			{		
				Node* thisNode = _scene->findNode(nodeID);
				Model* thisModel = static_cast<Model*>(thisNode->getDrawable());
				Material* newMaterial = nullptr;
				OurMaterial& ourMat = ourMaterialMap[materialName];

				//check if material use texture or color
				if (ourMat.diffuseTexFilePath.size() > 0)
					newMaterial = Material::create("res/shaders/textured.vert", "res/shaders/textured.frag", "POINT_LIGHT_COUNT 1");
				else
					newMaterial = Material::create("res/shaders/colored.vert", "res/shaders/colored.frag", "POINT_LIGHT_COUNT 1");
					
				RenderState::StateBlock* block = RenderState::StateBlock::create();
				block->setCullFace(true);
				block->setDepthTest(true);
				newMaterial->setStateBlock(block);
				newMaterial->setParameterAutoBinding("u_worldViewMatrix", RenderState::AutoBinding::WORLD_VIEW_MATRIX);
				newMaterial->setParameterAutoBinding("u_worldViewProjectionMatrix", RenderState::AutoBinding::WORLD_VIEW_PROJECTION_MATRIX);
				newMaterial->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", RenderState::AutoBinding::INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX);

				Node* lightNode = _scene->findNode("pointLightShape1");
				newMaterial->getParameter("u_pointLightColor[0]")->bindValue(lightNode->getLight(), &Light::getColor);
				newMaterial->getParameter("u_pointLightRangeInverse[0]")->bindValue(lightNode->getLight(), &Light::getRangeInverse);
				newMaterial->getParameter("u_pointLightPosition[0]")->bindValue(lightNode, &Node::getTranslationView);
				if (ourMat.diffuseTexFilePath.size() > 0)
					newMaterial->getParameter("u_diffuseTexture")->bindValue(&ourMat, &OurMaterial::getTexure);
				else
					newMaterial->getParameter("u_diffuseColor")->bindValue(&ourMat, &OurMaterial::getDiffuseColor);

				newMaterial->getParameter("u_ambientColor")->bindValue(&ourMat, &OurMaterial::getAmbientColor);

				thisModel->setMaterial(newMaterial);

				NodeIDToMaterial[nodeID] = materialName;
			}

			delete[] meshName;
			delete[] materialName;
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

		// Check if a new camera is created
		else if (type == MessageType::mCamera)
		{
			char* name;
			float camMatrix[4][4];
			bool isOrtho = true;
			float nearPlane = 0;
			float farPlane = 0;
			float aspectRatio = 0;
			float fov = 0;
			
			mayaData.getNewCamera(name, camMatrix, &isOrtho, &nearPlane, &farPlane, &aspectRatio, &fov);

			bool isNew = false;
			Node* cameraNode = _scene->findNode(name);
			if (!cameraNode)
			{
				cameraNode = Node::create(name);
				isNew = true;
			}

			if (cameraNode)
			{

				Matrix projectionMatrix(camMatrix[0][0], camMatrix[1][0], camMatrix[2][0], camMatrix[3][0],
					camMatrix[0][1], camMatrix[1][1], camMatrix[2][1], camMatrix[3][1],
					camMatrix[0][2], camMatrix[1][2], camMatrix[2][2], camMatrix[3][2],
					camMatrix[0][3], camMatrix[1][3], camMatrix[2][3], camMatrix[3][3]);

				Camera* cam;
				
				if (isOrtho)
					cam = Camera::createOrthographic(0, 0, aspectRatio, nearPlane, farPlane);
				else
					cam = Camera::createPerspective(MATH_RAD_TO_DEG(fov), aspectRatio, nearPlane, farPlane);
				//cam->setProjectionMatrix(projectionMatrix);

				cameraNode->setCamera(cam);

				if (isNew)
					_scene->addNode(cameraNode);
				
				if (_scene->getActiveCamera()->getNode() == NULL)
					_scene->setActiveCamera(cam);

				
			}
			delete[] name;
		}

		// Check if a camera attribute has changed
		else if (type == MessageType::mCameraChanged)
		{
			char* name;
			mayaData.getCameraChanged(name);

			Node* camNode = _scene->findNode(name);
			if (camNode)
				_scene->setActiveCamera(camNode->getCamera());
		}
		else if (type == MessageType::mLight)
		{
			float color[3];
			float range;
			mayaData.getLight(color, range);

			_scene->findNode("pointLightShape1")->getLight()->setColor(Vector3(color[0], color[1], color[2]));
			_scene->findNode("pointLightShape1")->getLight()->setRange(range);
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
}

void main::render(float elapsedTime)
{
    // Clear the color and depth buffers
    clear(CLEAR_COLOR_DEPTH, Vector4(0,0,0.2f,1), 1.0f, 0);

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

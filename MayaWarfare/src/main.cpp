#include "main.h"

// Declare our game instance
main game;

main::main()
    : _scene(NULL), _wireframe(false)
{
}

Vector4 main::getBLAH() const {
	return Vector4();
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

	boxMaterial->getParameter("u_diffusecolor")->bindValue(this, &main::getBLAH);

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
	tmpNode->release();
	
	Node* lightNode = Node::create("pointLightShape1");
	Light* light = Light::createPoint(Vector3(0.5f, 0.5f, 0.5f), 20);
	lightNode->setLight(light);
	lightNode->translate(Vector3(0, 0, 0));
	_scene->addNode(lightNode);
	lightNode->release();
	light->release();

	Node* dummyNode = Node::create("dummyNode");
	Mesh* dummyMesh = Mesh::createQuad(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 0, 0));
	Model* dummyModel = Model::create(dummyMesh);
	dummyNode->setDrawable(dummyModel);
	_scene->addNode(dummyNode);
	dummyNode->release();
	dummyMesh->release();
	dummyModel->release();

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

			//----- MATERIAL TEST--------------
			//if (!material)
			//{
				//material = Material::create("res/shaders/textured.vert", "res/shaders/textured.frag", "POINT_LIGHT_COUNT 1");
				//RenderState::StateBlock* block = RenderState::StateBlock::create();
				//block->setCullFace(true);
				//block->setDepthTest(true);
				//block->setDepthWrite(true);
				//material->setStateBlock(block);

				////Texture* tex = Texture::create(texturePath);
				////Node* lightNode = _scene->findNode("pointLightShape1");
				////Light* myLight = lightNode->getLight();

				////Bindings for vertex-shader

				//material->setParameterAutoBinding("u_worldViewMatrix", RenderState::AutoBinding::WORLD_VIEW_MATRIX);
				//material->setParameterAutoBinding("u_worldViewProjectionMatrix", RenderState::AutoBinding::WORLD_VIEW_PROJECTION_MATRIX);
				//material->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", RenderState::AutoBinding::INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX);

				//Light* light = _scene->findNode("pointLightShape1")->getLight();
				////material->getParameter("u_pointLightColor[0]")->bindValue(light, &Light::getColor);
				////material->getParameter("u_pointLightRangeInverse[0]")->bindValue(light, &Light::getRangeInverse);
				////material->getParameter("u_pointLightPosition[0]")->bindValue(light->getNode(), &Node::getTranslationView);
				//material->getParameter("u_pointLightColor[0]")->setValue(Vector3(1.0f, 1.0f, 1.0f));
				//material->getParameter("u_pointLightRangeInverse[0]")->setValue(1.0f / 50.0f);
				//material->getParameter("u_pointLightPosition[0]")->setValue(Vector3(1.0f, 1.0f, 1.0f));


				////material->getParameter("u_ambientColor")->setValue(Vector3(0.1f, 0.1f, 0.1f));
				////material->getParameter("u_pointLightColor[0]")->setValue(Vector3(1, 0, 0));//->bindValue(_scene->findNode("pointLightShape1")->getLight(), &Light::getColor);
				////material->getParameter("u_pointLightRange[0]")->bindValue(_scene->findNode("pointLightShape1")->getLight(), &Light::getRange);
				////material->getParameter("u_pointLightPosition[0]")->bindValue(_scene->findNode("pointLightShape1"), &Node::getTranslationView);
				////material->getParameter("u_pointLightRangeInverse[0]")->bindValue(_scene->findNode("pointLightShape1")->getLight(), &Light::getRangeInverse);
				////Bindings for fragment shader
				////material->setParameterAutoBinding("u_ambientColor", RenderState::AutoBinding::SCENE_AMBIENT_COLOR);
				////material->getParameter("u_diffuseColor")->setValue(Vector4(0.3f, 0.3f, 0.3f, 0));
			//}
			//----- MATERIAL TEST END---------

			Model* triModel = Model::create(triMesh);
			//triNode->setLight(_scene->findNode("pointLightShape1")->getLight());
			if (material)
				triModel->setMaterial(material);
			triNode->setDrawable(triModel);

			//lightNode->setDrawable(triModel);
			//_scene->addNode(lightNode);

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
		else if (type == MessageType::mNewMaterial)
		{
			char* name;
			char* texturePath;
			float diffuseColor[3];
			mayaData.getMaterial(name, texturePath, diffuseColor);

			bool modelsNeedNewMaterial = false;

			OurMaterial& ThisOurMaterial = ourMaterialMap[name];

			//Check if material used Texture before but uses Color now. Then materials need updates in models.
			if (ThisOurMaterial.diffuseTexFilePath.size() > 0 && !texturePath)
			{		
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
				ThisOurMaterial.diffuseTexFilePath = texturePath;
				ThisOurMaterial.texture = Texture::Sampler::create(texturePath, false);
				ThisOurMaterial.texture->setFilterMode(Texture::LINEAR, Texture::LINEAR);
				ThisOurMaterial.texture->setWrapMode(Texture::CLAMP, Texture::CLAMP);
			}
			else
				ourMaterialMap[name].color.set(diffuseColor[0], diffuseColor[1], diffuseColor[2], 1);

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
						block->setCullFaceSide(RenderState::CullFaceSide::CULL_FACE_SIDE_BACK);
						block->setDepthTest(true);
						newMaterial->setStateBlock(block);
						newMaterial->setParameterAutoBinding("u_worldViewMatrix", RenderState::AutoBinding::WORLD_VIEW_MATRIX);
						newMaterial->setParameterAutoBinding("u_worldViewProjectionMatrix", RenderState::AutoBinding::WORLD_VIEW_PROJECTION_MATRIX);
						newMaterial->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", RenderState::AutoBinding::INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX);

						Light* light = _scene->findNode("pointLightShape1")->getLight();
						newMaterial->getParameter("u_pointLightColor[0]")->setValue(Vector3(1.0f, 1.0f, 1.0f));
						newMaterial->getParameter("u_pointLightRangeInverse[0]")->setValue(1.0f / 50.0f);
						newMaterial->getParameter("u_pointLightPosition[0]")->setValue(Vector3(1.0f, 1.0f, 1.0f));
						if (texturePath)
							newMaterial->getParameter("u_diffuseTexture")->bindValue(&ourMaterialMap[name], &OurMaterial::getTexure);
						else
							newMaterial->getParameter("u_diffuseColor")->bindValue(&ourMaterialMap[name], &OurMaterial::getColor);

						newMaterial->getParameter("u_ambientColor")->setValue(Vector3(0.1f, 0.1f, 0.1f));
						
						Node* thisNode = _scene->findNode(it->first);
						Model* thisModel = static_cast<Model*>(thisNode->getDrawable());
						thisModel->setMaterial(newMaterial);
					}
				}
			}

			//----------------TEST-----------------------
			//Material* thisMaterial = materialMap[name];// = thisModel->getMaterial();
			//
			//bool isNew = false;
			//
			////If the material doesn't extist
			//if (!thisMaterial)
			//{
			//	//Create new material that will render Texture or Color
			//	if (texturePath)
			//	{
			//		thisMaterial = Material::create("res/shaders/textured.vert", "res/shaders/textured.frag", "POINT_LIGHT_COUNT 1");
			//		isNew = true;
			//	}
			//	else
			//	{
			//		thisMaterial = Material::create("res/shaders/colored.vert", "res/shaders/colored.frag", "POINT_LIGHT_COUNT 1");
			//		isNew = true;
			//	}
			//}
			//else //if Malterial alredy exist
			//{
			//	//check if material got color insted of texture
			//	if (ThisOurMaterial.diffuseTexFilePath.size() > 0 && !texturePath)
			//	{		
			//		thisMaterial = Material::create("res/shaders/colored.vert", "res/shaders/colored.frag", "POINT_LIGHT_COUNT 1");
			//		ThisOurMaterial.diffuseTexFilePath = "";
			//		ThisOurMaterial.texture->release();
			//		isNew = true;
			//	}
			//	//check material got texture insted of color
			//	else if (ThisOurMaterial.diffuseTexFilePath.size() == 0 && texturePath)
			//	{
			//		thisMaterial = Material::create("res/shaders/textured.vert", "res/shaders/textured.frag", "POINT_LIGHT_COUNT 1");
			//		isNew = true;
			//	}
			//	
			//}

			//if (texturePath)
			//{
			//	ourMaterialMap[name].diffuseTexFilePath = texturePath;
			//	ourMaterialMap[name].texture->release();
			//	ourMaterialMap[name].texture = Texture::Sampler::create(texturePath, false);
			//	ourMaterialMap[name].texture->setFilterMode(Texture::LINEAR, Texture::LINEAR);
			//	ourMaterialMap[name].texture->setWrapMode(Texture::CLAMP, Texture::CLAMP);
			//}
			//else
			//	ourMaterialMap[name].color.set(diffuseColor[0], diffuseColor[1], diffuseColor[2], 1);

			//if (isNew)
			//{
			//	RenderState::StateBlock* block = RenderState::StateBlock::create();
			//	block->setCullFace(true);
			//	block->setCullFaceSide(RenderState::CullFaceSide::CULL_FACE_SIDE_BACK);
			//	block->setDepthTest(true);
			//	thisMaterial->setStateBlock(block);
			//	thisMaterial->setParameterAutoBinding("u_worldViewMatrix", RenderState::AutoBinding::WORLD_VIEW_MATRIX);
			//	thisMaterial->setParameterAutoBinding("u_worldViewProjectionMatrix", RenderState::AutoBinding::WORLD_VIEW_PROJECTION_MATRIX);
			//	thisMaterial->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", RenderState::AutoBinding::INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX);

			//	Light* light = _scene->findNode("pointLightShape1")->getLight();
			//	thisMaterial->getParameter("u_pointLightColor[0]")->setValue(Vector3(1.0f, 1.0f, 1.0f));
			//	thisMaterial->getParameter("u_pointLightRangeInverse[0]")->setValue(1.0f / 50.0f);
			//	thisMaterial->getParameter("u_pointLightPosition[0]")->setValue(Vector3(1.0f, 1.0f, 1.0f));
			//	if (texturePath)
			//		thisMaterial->getParameter("u_diffuseTexture")->bindValue(&ourMaterialMap[name], &OurMaterial::getTexure);
			//	else
			//		thisMaterial->getParameter("u_diffuseColor")->bindValue(&ourMaterialMap[name], &OurMaterial::getColor);
			//
			//	thisMaterial->getParameter("u_ambientColor")->setValue(Vector3(0.1f, 0.1f, 0.1f));

			//	materialMap[name] = thisMaterial;

			//	for (std::vector<char*>::iterator it = materialToNodeID[name].begin(); it != materialToNodeID[name].end(); it++)
			//	{
			//		Node* thisNode = _scene->findNode(*it);
			//		Model* thisModel = static_cast<Model*>(thisNode->getDrawable());
			//		thisModel->setMaterial(thisMaterial);
			//	}
			//}

	

			//----------------TEST-----------------------

			//material->getParameter("u_directionalLightColor[0]")->bindValue()
			
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
				//bool needNewMaterail = true;
				//for (std::vector<char*>::iterator it = materialToNodeID[materialName].begin(); it != materialToNodeID[materialName].end(); it++)
				//{
				//	if (*it == nodeID)
				//	{
				//		needNewMaterail = false;
				//		break;
				//	}
				//}
				//if (needNewMaterail)
				//{
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

					Light* light = _scene->findNode("pointLightShape1")->getLight();
					newMaterial->getParameter("u_pointLightColor[0]")->setValue(Vector3(1.0f, 1.0f, 1.0f));
					newMaterial->getParameter("u_pointLightRangeInverse[0]")->setValue(1.0f / 50.0f);
					newMaterial->getParameter("u_pointLightPosition[0]")->setValue(Vector3(1.0f, 1.0f, 1.0f));
					if (ourMat.diffuseTexFilePath.size() > 0)
						newMaterial->getParameter("u_diffuseTexture")->bindValue(&ourMat, &OurMaterial::getTexure);
					else
						newMaterial->getParameter("u_diffuseColor")->bindValue(&ourMat, &OurMaterial::getColor);

					newMaterial->getParameter("u_ambientColor")->setValue(Vector3(0.1f, 0.1f, 0.1f));

					thisModel->setMaterial(newMaterial);

					NodeIDToMaterial[nodeID] = materialName;
				//}
				//std::string oldMaterialName;

				////Check if it has an old material
				//if (thisMaterial)
				//{
				//	//get old Material name
				//	for (std::map<std::string, Material*>::iterator it = materialMap.begin(); it != materialMap.end(); it++)
				//	{
				//		if (it->second == thisMaterial)
				//		{
				//			oldMaterialName = it->first;
				//			break;
				//		}
				//	}
				//	UINT i = 0;
				//	//Delete this node from material list
				//	for (std::vector<char*>::iterator it = materialToNodeID[oldMaterialName].begin(); it != materialToNodeID[oldMaterialName].end(); it++)
				//	{
				//		if (*it == nodeID)
				//		{
				//			materialToNodeID[oldMaterialName].erase(materialToNodeID[oldMaterialName].begin() + i);
				//			break;
				//		}
				//		i++;
				//	}
				//}

				//thisModel->setMaterial(materialMap[materialName]);
				//materialToNodeID[materialName].push_back(nodeID);
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
			delete[] name;
			
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


	// Rotate model
	_scene->findNode("box")->translateX(0.01f);
	_scene->findNode("box")->rotateX(0.01f);

	for(std::vector<char*>::iterator it = nodeNames.begin(); it != nodeNames.end(); ++it)
	{
		//_scene->findNode(*it)->rotateX(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));
		//_scene->findNode(*it)->translateX(-0.001f * i);
		//i *= -1;
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

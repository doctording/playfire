﻿#include "BgScene.h"
#include "GameOver.h"

USING_NS_CC;

Scene* Bg::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = Bg::create();

	// add layer as a child to scene
	scene->addChild(layer);

	return scene;
}

bool Bg::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}
	visibleSize = Director::getInstance()->getVisibleSize();
	origin = Director::getInstance()->getVisibleOrigin();

	/*auto sp = Sprite::create("CloseNormal.png");
	sp->setPosition(Point(visibleSize.width / 2, visibleSize.height / 2));
	this->addChild(sp,1);*/

	initBG();

	// 添加飞机
	planeLayer = PlaneLayer::create();
	this->addChild(planeLayer, 1);

	this->scheduleUpdate(); // 更新

	// 键盘事件
	auto listenerKeyPad = EventListenerKeyboard::create();
	listenerKeyPad->onKeyPressed = CC_CALLBACK_2(Bg::onKeyPressed, this);
	listenerKeyPad->onKeyReleased = CC_CALLBACK_2(Bg::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listenerKeyPad, this);

	// 按键是否按下
	isAdown = false;
	isDdown = false;

	// 添加敌机
	this->schedule(SEL_SCHEDULE(&Bg::addEnemy), 1.0f);

/**/
	//创建监听事件对象
	auto listener = EventListenerTouchOneByOne::create();
	//定义监听事件的回调函数
	listener->onTouchBegan = CC_CALLBACK_2(Bg::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(Bg::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(Bg::onTouchEnded, this);
	//在事件分发器中注册
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);


	return true;
}

void Bg::initBG()
{
	m_bgSprite1 = Sprite::create("background.png");
	m_bgSprite1->setPosition(Point(visibleSize.width / 2, visibleSize.height / 2));
	this->addChild(m_bgSprite1, 0);

	m_bgSprite2 = Sprite::create("background.png");
	m_bgSprite2->setPosition(Point(visibleSize.width / 2, visibleSize.height + visibleSize.height / 2));
	this->addChild(m_bgSprite2, 0);

	// 添加上scoreLabel
	scoreLabel = Label::create("Score: 0", "Arial", 24);
	this->score = 0;
	scoreLabel->setPosition(Point(origin.x + visibleSize.width - 100,
		origin.y + visibleSize.height - scoreLabel->getContentSize().height));
	this->addChild(scoreLabel, 2);
}

void Bg::update(float delta)
{
	int posY1 = m_bgSprite1->getPositionY();	//背景地图1的Y坐标
	int posY2 = m_bgSprite2->getPositionY();	//背景地图2的Y坐标

	int iSpeed = 1;	//地图滚动速度

	/* 两张地图向左滚动（两张地图是相邻的，所以要一起滚动，否则会出现空隙） */
	posY1 -= iSpeed;
	posY2 -= iSpeed;

	/* 图大小 */
	Size bgSize = m_bgSprite1->getContentSize();

	/* 当第1个地图完全离开屏幕时，第2个地图刚好完全出现在屏幕上，这时候就让第1个地图紧贴在
	 第2个地图后面 */
	if (posY1 <= -bgSize.height / 2) {
		posY1 = bgSize.height + bgSize.height / 2;
	}
	/* 同理，当第2个地图完全离开屏幕时，第1个地图刚好完全出现在屏幕上，这时候就让第2
	 个地图紧贴在第1个地图后面 */
	if (posY2 <= -bgSize.height / 2) {
		posY2 = bgSize.height + bgSize.height / 2;
	}

	m_bgSprite1->setPositionY(posY1);
	m_bgSprite2->setPositionY(posY2);



	// 飞机移动
	if (isAdown)
		this->planeLayer->moveLeftOrRight('A');
	if (isDdown)
		this->planeLayer->moveLeftOrRight('D');

	auto & vector2 = Manager::getInstance()->getBulletVector();
	auto & vector1 = Manager::getInstance()->getEnemyVector();

	// 飞机是否被撞了
	for (EnemyBase* spenemy : vector1)
	{
		Rect r1 = spenemy->getBoundingBox();
		Rect r2 = planeLayer->plane->getBoundingBox();

		if (r1.intersectsRect(r2))
		{
			//CCLOG("GameOver");
			gameOver();
		}
	}

	Vector<Bullet*> rm1;
	Vector<EnemyBase*> rm2;

	// 判断敌机与子弹
	for (Bullet* spbullet : vector2)
	{
		Rect r1 = spbullet->getBoundingBox();
		for (EnemyBase* spenemy : vector1)
		{
			if (spenemy->getHp() == 0) // 敌机血量为0 已经死亡了
			{
				continue;
			}
			Rect r2 = spenemy->getBoundingBox();
			if (r1.intersectsRect(r2)) // 相撞
			{
				spenemy->setHp(spenemy->getHp() - 1);

				rm1.pushBack(spbullet); // 子弹消失
				spbullet->removeFromParentAndCleanup(true);

				this->score += 10; // 碰撞后的分数添加

				if (spenemy->getHp() == 0) // 判断敌机死亡并销毁
				{
					// spenemy->blowUp(); 好像没有什么效果
					rm2.pushBack(spenemy);
					spenemy->removeFromParentAndCleanup(true);
				}
				else{
					spenemy->hit();
				}

			}
		}
	}

	// 更新manager中的容器
	for (Bullet* spbullet : rm1)
		vector2.eraseObject(spbullet);
	for (EnemyBase* spenemy : rm2)
		vector1.eraseObject(spenemy);


	// 更新分数
	auto str = __String::createWithFormat("score: %d", score);
	scoreLabel->setString(str->getCString());
}

// 按键 按下
void Bg::onKeyPressed(EventKeyboard::KeyCode keycode, cocos2d::Event *event)
{
	switch (keycode)
	{
	case EventKeyboard::KeyCode::KEY_A:
		//Director::getInstance()->pause();
		//this->planeLayer->moveLeftOrRight('A');
		isAdown = true;
		break;
	case EventKeyboard::KeyCode::KEY_D:
		//this->planeLayer->moveLeftOrRight('D');
		isDdown = true;
		break;
	case EventKeyboard::KeyCode::KEY_W:

		break;
	case EventKeyboard::KeyCode::KEY_S:

		break;
	// I键暂停游戏，截屏并跳转
	case EventKeyboard::KeyCode::KEY_I:
	{
		RenderTexture *renTexture = RenderTexture::create(400, 550);
		renTexture->begin();
		this->getParent()->visit();
		renTexture->end();
		Director::getInstance()->pushScene(Pause::scene(renTexture, true));
	}
		break;

	// 用来发射子弹
	case EventKeyboard::KeyCode::KEY_J:
	{
		this->schedule(SEL_SCHEDULE(&Bg::addBullet), 0.08f); // 开启子弹发射
	}
	break;

	default:
		break;
	}
}

//按键 释放
void Bg::onKeyReleased(EventKeyboard::KeyCode keycode, cocos2d::Event *event)
{
	switch (keycode)
	{
	case EventKeyboard::KeyCode::KEY_A:
		isAdown = false;
		break;
	case EventKeyboard::KeyCode::KEY_D:
		isDdown = false;
		break;
	case EventKeyboard::KeyCode::KEY_W:
		break;
	case EventKeyboard::KeyCode::KEY_S:

		break;

	case EventKeyboard::KeyCode::KEY_J:
	{
		this->unschedule(SEL_SCHEDULE(&Bg::addBullet)); // 取消子弹定时器
	}
		break;

	default:
		break;
	}
}

void Bg::addBullet(float tm)
{
	auto bullet = Bullet::create();
	bullet->initBullet("AirplaneResource/ui/shoot/bullet1.png");
	// 设置字段的位置为飞机的出口处 
	bullet->setPosition(Point(planeLayer->plane->getPositionX(),
		planeLayer->plane->getPositionY() + planeLayer->plane->getContentSize().height / 2 + 10));
	// 添加到场景中 
	this->addChild(bullet, 1);

	// 这里还需要将子弹添加到管理器中
	auto & bulletVector = Manager::getInstance()->getBulletVector();
	bulletVector.pushBack(bullet);
}

void Bg::addEnemy(float tm)
{
	/*
		这里是根据一个分数或者说时间来改变敌机出现的快慢，先写死了，后期添加上了分数再来做这块
	*/
	static int score = 0;
	score++;
	if (score>50)
	{
		//重新调用schedule会更新时间的
		this->schedule(SEL_SCHEDULE(&Bg::addEnemy), 0.8f);
	}
	else if (score>100)
	{
		this->schedule(SEL_SCHEDULE(&Bg::addEnemy), 0.6f);
	}
	else if (score>200)
	{
		this->schedule(SEL_SCHEDULE(&Bg::addEnemy), 0.5f);
	}
	else if (score>400)
	{
		this->schedule(SEL_SCHEDULE(&Bg::addEnemy), 0.3f);
	}

	auto enemy = EnemyBase::create();
	//根据不同的概率随机的添加不同种类的飞机
	int enemy_x = CCRANDOM_0_1() * 9 + 1;
	int count = 0;
	if (enemy_x > 0 && enemy_x <7)
	{
		enemy_x = 1;
		count = 4;
	}
	else if (enemy_x >= 7 && enemy_x < 9)
	{
		enemy_x = 2;
		count = 4;
	}
	else
	{
		//敌机三用到的背景图片不太一样，这里单独的建立下
		count = 6;
		enemy->initEnemy("AirplaneResource/ui/shoot/enemy3_n1", count);
		//创建敌机三的动画
		Vector<SpriteFrame *> vector;
		//vector.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_n1.png"));
		//vector.pushBack(SpriteFrameCache::getInstance()->getSpriteFrameByName("enemy3_n2.png"));
		//auto animation = Animation::createWithSpriteFrames(vector, 0.2f, -1);
		//auto animate = Animate::create(animation);
		//enemy->runAction(animate);
		//添加到当前的层中
		this->addChild(enemy, 1);
		//直接返回，不再执行下面的语句
		return;
	}
	//以下的这句话一定要调用，添加敌机
	auto str = __String::createWithFormat("AirplaneResource/ui/shoot/enemy%d", enemy_x);
	enemy->initEnemy(str->getCString(), count);
	this->addChild(enemy, 1);

	// 创建的敌机添加到管理器中
	auto & enemyVector = Manager::getInstance()->getEnemyVector();
	enemyVector.pushBack(enemy);
}

//游戏结束
void Bg::gameOver()
{
	//停止发射子弹 等
	this->unschedule(SEL_SCHEDULE(&Bg::addBullet));
	this->unschedule(SEL_SCHEDULE(&Bg::addEnemy));

	this->unschedule(schedule_selector(Bg::update));

	//玩家飞机炸毁的动画
	//this->planeLayer->blow();

	// 根据图片帧制作动画
	Vector<SpriteFrame*> spriteFrameVec;
	auto spriteFrameCache = SpriteFrameCache::getInstance();

	char path[256] = { 0 };
	for (int i = 1; i <= 4; ++i)
	{
		sprintf(path, "AirplaneResource/ui/shoot/hero_blowup_n%d.png", i);

		// 150*105是图片的大小，貌似这样直接设定是不好的；但是还必须提供这个参数
		// 应该给这个Rect参数提供一个默认参数，默认加载整个图片
		SpriteFrame *pSpriteFrame = SpriteFrame::create(path, Rect(0, 0, 102, 126));

		// 加入SpriteFrameCache中
		spriteFrameCache->addSpriteFrame(pSpriteFrame, path);
		spriteFrameVec.pushBack(pSpriteFrame);
	}

	Animation* animation = Animation::createWithSpriteFrames(spriteFrameVec);
	animation->setLoops(1);
	animation->setRestoreOriginalFrame(false); // 设置是否在动画播放结束后恢复到第一帧
	// should last 2.0 seconds. And there are 4 frames.
	animation->setDelayPerUnit(1.3f / 4.0f);
	Animate* peeAnimate = Animate::create(animation);

	//切换场景
	auto func = [this]()
	{
		Manager::freeInstance(); // 销毁所有的子弹，敌机等数据
		auto scene = GameOver::createScene();
		GameOver *overLayer = (GameOver*)scene->getChildByTag(404); // tag
		overLayer->score_int = score;
		// 切换到 GameOver 场景
		Director::getInstance()->replaceScene(scene);
	};
	//执行动作序列
	auto sequence = Sequence::create(peeAnimate, CallFunc::create(func), NULL);
	this->planeLayer->plane->runAction(sequence);
}

bool Bg::onTouchBegan(Touch *touch, Event *unused_event)
{
	//获取触屏位置（坐标）
	Point pos = touch->getLocation();
	//CCLOG("TouchBegan");
	return true;
}

void Bg::onTouchMoved(Touch *touch, Event *unused_event)
{
	//获取当前拖动手势的坐标与位置
	Point pos = touch->getLocation();
	this->planeLayer->plane->setPosition(pos);
	this->schedule(SEL_SCHEDULE(&Bg::addBullet), 0.2f); // 开启子弹发射
	//CCLOG("TouchMoved");
}

void Bg::onTouchEnded(Touch *touch, Event *unused_event)
{
	this->unschedule(SEL_SCHEDULE(&Bg::addBullet)); // 取消子弹定时器
}
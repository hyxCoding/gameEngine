#include  "GameEngine.h"
#include <cmath>
//重力类方法--------------------------------------------------------------------------------
Gravity::Gravity(double GravityAccel,double MaxRisingSpeed,double MaxFallingSpeed):
gravityAccel(GravityAccel),maxRisingSpeed(MaxRisingSpeed),maxFallingSpeed(MaxFallingSpeed){
    this->parent=nullptr;
}
Gravity::~Gravity(){}
void Gravity::updateMoveStatus(){
    if(this->parent==nullptr){return;}
    if(this->active==false){//是否启用重力
        return;
    }
    double verticalSpeed=this->parent->getVerticalSpeed();
    if(this->reverse==false&&verticalSpeed>this->maxFallingSpeed
    ||this->reverse==true&&verticalSpeed<this->maxRisingSpeed){return;}//超出重力可控范围
    double addSpeed=this->parent->getUnitTime() * this->gravityAccel * (this->reverse==false?DOWNDIR:UPDIR);
    this->parent->getVerticalSpeed()+=addSpeed;

    if(this->reverse==false&&this->parent->getVerticalSpeed() > this->maxFallingSpeed)
    {this->parent->getVerticalSpeed() = this->maxFallingSpeed;}//下落最大速度判断

    if(this->reverse==true&&this->parent->getVerticalSpeed() < this->maxRisingSpeed)
    {this->parent->getVerticalSpeed() = this->maxRisingSpeed;}//上升最大速度判断
    return;
}

//水平摩擦力类方法
HorizontalFriction::HorizontalFriction(double FrictionAccel):
frictionAccel(FrictionAccel){
    this->parent=nullptr;
}

HorizontalFriction::~HorizontalFriction(){

}

void HorizontalFriction::updateMoveStatus(){//更新运动状态
    if(this->parent==nullptr){return;}
    double horizontalSpeed=this->parent->getHorizontalSpeed();
    if(this->active==false||horizontalSpeed < this->minOpenSpeedLimit
        &&horizontalSpeed> -this->minOpenSpeedLimit){
        return;
    }
    int frictionDir=horizontalSpeed>0?LEFTDIR:RIGHTDIR;//确定摩擦力方向
    frictionDir=this->reverse==true?-frictionDir:frictionDir;//反转
    double addSpeed=this->parent->getUnitTime() * this->frictionAccel * frictionDir;
    this->parent->getHorizontalSpeed()+=addSpeed;
    int moveDir=this->parent->getHorizontalSpeed() > 0?RIGHTDIR:LEFTDIR;//确定现在运动方向
    if(this->reverse==false && moveDir==frictionDir){//摩擦力和运动同向，即摩擦速度变化量过大
        this->parent->getHorizontalSpeed()=0;//速度归0
    }
    return;
}

//空气阻力方法
AirFriction::AirFriction(double FrictionAccel):frictionAccel(FrictionAccel){
    this->parent=nullptr;
}

AirFriction::~AirFriction(){

}

void AirFriction::updateMoveStatus(){
    if(this->parent==nullptr){return;}
    double verticalSpeed=this->parent->getVerticalSpeed();
    if(this->active==false||verticalSpeed > -this->minOpenSpeedLimit 
        &&verticalSpeed < this->minOpenSpeedLimit){//关闭或不在启动范围内
        return;
    }
    int frictionDir=verticalSpeed > 0?UPDIR:DOWNDIR;//确定摩擦方向
    frictionDir=this->reverse==true? -frictionDir : frictionDir;//反转
    double k=std::fabs(verticalSpeed)/(std::fabs(verticalSpeed) + 1);//k为空气阻力系数
    double addSpeed=this->parent->getUnitTime() * this->frictionAccel * k * frictionDir;
    this->parent->getVerticalSpeed()+=addSpeed;
    int moveDir=this->parent->getVerticalSpeed() > 0?DOWNDIR:UPDIR;//确定现在运动方向
    if(this->reverse==false && frictionDir==moveDir){//阻力不改变运动方向
        this->parent->getVerticalSpeed()=0;
    }
    return;
}

//行走类方法--------------------------------------------------------------------------------
Walk::Walk(double _maxWalkSpeed,double _walkAccel):
maxWalkSpeed(_maxWalkSpeed),walkAccel(_walkAccel){
    this->parent=nullptr;
}
Walk::~Walk(){

}
void Walk::updateMoveStatus(){
    if(this->parent==nullptr){return;}
    if(this->parent->getMove_Status().isWalk==false){//没走
        return;
    }
    bool isLeft=this->parent->getMove_Status().isLeft;
    bool isRight=this->parent->getMove_Status().isRight;
    if(isLeft==true && isRight==true){return;}
    if(this->parent->getHorizontalReverse()==true){//启用左右反转
        isLeft==this->parent->getMove_Status().isRight;
        isRight=this->parent->getMove_Status().isLeft;
    }
    if(isLeft==true&&this->parent->getHorizontalSpeed() <= this->maxWalkSpeed * LEFTDIR
    ||isRight==true&&this->parent->getHorizontalSpeed() >= this->maxWalkSpeed * RIGHTDIR){//超出最大自主可控水平速度范围
        return;
    }
    double addSpeed=this->parent->getUnitTime() * this->walkAccel * (isLeft==true?LEFTDIR:RIGHTDIR);
    this->parent->getHorizontalSpeed() += addSpeed;
    if(isLeft==true&&this->parent->getHorizontalSpeed() < this->maxWalkSpeed * LEFTDIR){//超出左移速度可控范围
        this->parent->getHorizontalSpeed()=this->maxWalkSpeed * LEFTDIR;
    }
    if(isRight==true&&this->parent->getHorizontalSpeed() > this->maxWalkSpeed * RIGHTDIR){//超出右移速度可控范围
        this->parent->getHorizontalSpeed()=this->maxWalkSpeed * RIGHTDIR;
    }
    this->parent->getImage()->playWalkAnimation();
    return;
}

//跳跃类方法--------------------------------------------------------------------------
Jump::Jump(int JumpCnt,int MaxJumpCnt,double JumpSpeed,double MaxJumpSpeed):
jumpCnt(JumpCnt),jumpSpeed(JumpSpeed),maxJumpCnt(MaxJumpCnt),maxJumpSpeed(MaxJumpSpeed){
    this->parent=nullptr;
}
Jump::~Jump(){

}
void Jump::updateMoveStatus(){
    if(this->parent==nullptr){return;}
    if(this->parent->getMove_Status().isJump==false){//根本没跳
        return;
    }
    if(this->jumpCnt>=this->maxJumpCnt){//达到最大连续跳跃次数
        return;
    }
    if(this->parent->getVerticalSpeed() < this->maxJumpSpeed){//超出跳跃可控的速度范围
        return;
    }
    this->parent->getVerticalSpeed() += this->jumpSpeed;
    if(this->parent->getVerticalSpeed() < this->maxJumpSpeed){//去除超出部分
        this->parent->getVerticalSpeed()=this->maxJumpSpeed;
    }
    this->jumpCnt++;
    this->parent->getImage()->playJumpAnimation();
    return;
}

//挂机类方法---------------------------------------------------------------------------------------
Idle::Idle(){
    this->parent=nullptr;
}

void Idle::updateMoveStatus(){
    if(this->parent==nullptr){return;}
    if(this->parent->getMove_Status().isIdle==false){//没挂机
        return;
    }
    this->parent->getVerticalSpeed()=0;//速度置0
    this->parent->getHorizontalSpeed()=0;
    this->parent->getImage()->playIdleAnimation();
}

Idle::~Idle(){

}

//奔跑类方法----------------------------------------------------------------------------------------
Run::Run(double _runAccel,double _maxRunSpeed):runAccel(_runAccel),maxRunSpeed(_maxRunSpeed){
    this->parent=nullptr;
}

void Run::updateMoveStatus(){
    if(this->parent==nullptr){return;}
    if(this->parent->getMove_Status().isRun==false){//没奔跑
        return;
    }
    bool isLeft=this->parent->getMove_Status().isLeft;
    bool isRight=this->parent->getMove_Status().isRight;
    if(isLeft==true && isRight==true){return;}
    if(this->parent->getHorizontalReverse()==true){//开启左右反转
        isLeft=this->parent->getMove_Status().isRight;
        isRight=this->parent->getMove_Status().isLeft;
    }
    double horizontalSpeed=this->parent->getHorizontalSpeed();
    if(isLeft==true && horizontalSpeed <= this->maxRunSpeed * LEFTDIR
    ||isRight==true && horizontalSpeed >= this->maxRunSpeed * RIGHTDIR){//超出可控范围
        return;
    }
    double addSpeed=this->parent->getUnitTime() * this->runAccel * (isLeft==true?LEFTDIR:RIGHTDIR);
    this->parent->getHorizontalSpeed() += addSpeed;
    if(isLeft==true&&this->parent->getHorizontalSpeed() < this->maxRunSpeed * LEFTDIR){//超出左移速度可控范围
        this->parent->getHorizontalSpeed()=this->maxRunSpeed * LEFTDIR;
    }
    if(isRight==true&&this->parent->getHorizontalSpeed() > this->maxRunSpeed * RIGHTDIR){//超出右移速度可控范围
        this->parent->getHorizontalSpeed()=this->maxRunSpeed * RIGHTDIR;
    }
    this->parent->getImage()->playRunAnimation();
    return;
}

Run::~Run(){

}

//冲刺类方法---------------------------------------------------------------------------------------
Sprint::Sprint(double _sprintAccel,double _maxSprintSpeed):
sprintAccel(_sprintAccel),maxSprintSpeed(_maxSprintSpeed){
    this->parent=nullptr;
}

void Sprint::updateMoveStatus(){
    if(this->parent==nullptr){return;}
    if(this->parent->getMove_Status().isSprint==false){//没冲刺
        return;
    }
    bool isLeft=this->parent->getMove_Status().isLeft;
    bool isRight=this->parent->getMove_Status().isRight;
    if(isLeft==true && isRight==true){return;}
    if(this->parent->getHorizontalReverse()==true){//开启左右反转
        isLeft=this->parent->getMove_Status().isRight;
        isRight=this->parent->getMove_Status().isLeft;
    }
    double horizontalSpeed=this->parent->getHorizontalSpeed();
    if(isLeft==true && horizontalSpeed <= this->maxSprintSpeed * LEFTDIR
    ||isRight==true && horizontalSpeed >= this->maxSprintSpeed * RIGHTDIR){//超出可控范围
        return;
    }
    double addSpeed=this->parent->getUnitTime() * this->sprintAccel * (isLeft==true?LEFTDIR:RIGHTDIR);
    this->parent->getHorizontalSpeed() += addSpeed;
    if(isLeft==true&&this->parent->getHorizontalSpeed() < this->maxSprintSpeed * LEFTDIR){//超出左移速度可控范围
        this->parent->getHorizontalSpeed()=this->maxSprintSpeed * LEFTDIR;
    }
    if(isRight==true&&this->parent->getHorizontalSpeed() > this->maxSprintSpeed * RIGHTDIR){//超出右移速度可控范围
        this->parent->getHorizontalSpeed()=this->maxSprintSpeed * RIGHTDIR;
    }
    this->parent->getImage()->playSprintAnimation();
    return;
}

Sprint::~Sprint(){

}

//下落类方法----------------------------------------------------------------------------------------
Fall::Fall(){
    this->parent=nullptr;
}

void Fall::updateMoveStatus(){
    if(this->parent==nullptr){return;}
    if(this->parent->getMove_Status().isFall==false){
        return;
    }
    this->parent->getImage()->playFallAnimation();
    return;
}

Fall::~Fall(){

}

//着地类方法----------------------------------------------------------------------------------------
Land::Land(){
    this->parent=nullptr;
}

void Land::updateMoveStatus(){
    if(this->parent==nullptr){return;}
    if(this->parent->getMove_Status().isLand==false){
        return;
    }
    this->parent->getVerticalSpeed()=0;
    this->parent->getHorizontalSpeed()=0;
    this->parent->getJump()->resetJumpCnt();//跳跃次数清零
    this->parent->getImage()->playLandAnimation();
    return;
}

Land::~Land(){

}

//下蹲类方法-------------------------------------------------------------------------------------
Crouch::Crouch(){
    this->parent=nullptr;
}

void Crouch::updateMoveStatus(){
    if(this->parent==nullptr){return;}
    if(this->parent->getMove_Status().isCrouch==false){
        return;
    }
    this->parent->getImage()->playCrouchAnimation();
    return;
}

Crouch::~Crouch(){

}

//普通攻击类方法-----------------------------------------------------------------------------------
Attack::Attack(){
    this->parent=nullptr;
}

void Attack::updateMoveStatus(){
    if(this->parent==nullptr){return;}
    if(this->parent->getMove_Status().isAttack==false){
        return;
    }
    this->parent->getImage()->playAttackAnimation();
    return;
}

Attack::~Attack(){

}

//受伤类方法----------------------------------------------------------------------------------------
Hurt::Hurt(){
    this->parent=nullptr;
}

void Hurt::updateMoveStatus(){
    if(this->parent==nullptr){return;}
    if(this->parent->getMove_Status().isHurt==false){
        return;
    }
    this->parent->getImage()->playHurtAnimation();
    return;
}

Hurt::~Hurt(){

}

//阵亡类方法---------------------------------------------------------------------------------------
Death::Death(){
    this->parent=nullptr;
}

void Death::updateMoveStatus(){
    if(this->parent==nullptr){return;}
    if(this->parent->getMove_Status().isDeath==false){
        return;
    }
    this->parent->getImage()->playDeathAnimation();
    return;
}

Death::~Death(){

}

//运动管理类方法------------------------------------------------------------------------------------
MoveManager::MoveManager(PointF* _curPos,double* _verticalSpeed,double* _horizontalSpeed,int _unitTime):
curPos(_curPos),verticalSpeed(_verticalSpeed),horizontalSpeed(_horizontalSpeed),unitTime(_unitTime){
    this->jump=nullptr;
    this->walk=nullptr;
    this->move=nullptr;
    this->idle=nullptr;
    this->run=nullptr;
    this->sprint=nullptr;
    this->fall=nullptr;
    this->land=nullptr;
    this->crouch=nullptr;
    this->attack=nullptr;
    this->hurt=nullptr;
    this->death=nullptr;
    this->image=nullptr;
}

void MoveManager::updateMoveStatus(){
    if(this->jump != nullptr){
        this->jump->updateMoveStatus();
    }
    if(this->walk != nullptr){
        this->walk->updateMoveStatus();
    }
    if(this->idle != nullptr){
        this->idle->updateMoveStatus();
    }
    if(this->run != nullptr){
        this->run->updateMoveStatus();
    }
    if(this->sprint != nullptr){
        this->sprint->updateMoveStatus();
    }
    if(this->fall != nullptr){
        this->fall->updateMoveStatus();
    }
    if(this->land != nullptr){
        this->land->updateMoveStatus();
    }
    if(this->crouch != nullptr){
        this->crouch->updateMoveStatus();
    }
    if(this->attack != nullptr){
        this->attack->updateMoveStatus();
    }
    if(this->hurt != nullptr){
        this->hurt->updateMoveStatus();
    }
    if(this->death != nullptr){
        this->death->updateMoveStatus();
    }
    if(this->move != nullptr){
        this->move->updatePos();
    }
    return;
}

bool MoveManager::isLand(){
    bool ans= *(this->verticalSpeed) > 0 && 
    this->move_status.isFall==false ? true : false;//有速度且没下落
    return ans;
}

MoveManager::~MoveManager(){

}

//物理计算管理类方法--------------------------------------------------------------------------------
PhysicsManager::PhysicsManager(double* _verticalSpeed,double* _horizontalSpeed,int _unitTime):
verticalSpeed(_verticalSpeed),horizontalSpeed(_horizontalSpeed),unitTime(_unitTime){
    this->gravity=nullptr;
    this->airFriction=nullptr;
    this->horizontalFriction=nullptr;
}

void PhysicsManager::updatePhysicsStatus(){
    if(this->gravity != nullptr){
        this->gravity->updateMoveStatus();
    }
    if(this-> airFriction != nullptr){
        this->airFriction->updateMoveStatus();
    }
    if(this->horizontalFriction != nullptr){
        this->horizontalFriction->updateMoveStatus();
    }
    return;
}

PhysicsManager::~PhysicsManager(){

}

//人物血量和buff管理类方法-----------------------------------------------------------
HealthAndBuffManager::HealthAndBuffManager(CharacterData _characterData):
characterData(_characterData){

}

HealthAndBuffManager::~HealthAndBuffManager(){

}

//玩家控制类方法--------------------------------------------------------------------
PlayerController::PlayerController
(PointF _curPos,double _verticalSpeed,double _horizontalSpeed,CharacterData _characterData)
:curPos(_curPos),verticalSpeed(_verticalSpeed),horizontalSpeed(_horizontalSpeed),
physicsManager(&verticalSpeed,&horizontalSpeed),
moveManager(&curPos,&verticalSpeed,&horizontalSpeed,unitTime),healthAndBuffManager(_characterData)
{
    this->image=nullptr;
    this->dynamicCollisionObject=nullptr;
    this->isExist=true;
}

void PlayerController::updateAllStatus(){
    this->updateKeyStatus();
    this->physicsManager.updatePhysicsStatus();
    this->moveManager.updateMoveStatus();
    return;
}

void PlayerController::addImage(IMAGE* img,int _priority){
    this->image=new Image(PointF(0,0),img,_priority);
    this->image->setPos(&this->curPos);
    this->moveManager.setImage(this->image);
    return;
}

void PlayerController::addDynamicCollisionObject(DynamicCollisionObject* _dynamicCollisionObject){
    this->dynamicCollisionObject=_dynamicCollisionObject;
    return;
}

void PlayerController::
addDynamicCollisionObject(PointF* _curPos,bool* _isExist,int _width,int _height,int _grid_width,int _grid_height){
    this->dynamicCollisionObject=new DynamicCollisionObject(_curPos,_isExist,_width,_height,_grid_width,_grid_height);
    return;
}

PlayerController::~PlayerController(){

}

//移动类方法---------------------------------------------------------------------------
Move::Move(){
    this->parent=nullptr;
}

void Move::updatePos(){
    double next_x=this->parent->getCurPos().getX() + 
    this->parent->getUnitTime() * this->parent->getHorizontalSpeed();
    double next_y=this->parent->getCurPos().getY() + 
    this->parent->getUnitTime() * this->parent->getVerticalSpeed();
    this->parent->getCurPos().setX(next_x);
    this->parent->getCurPos().setY(next_y);
    return;
}

Move::~Move(){

}

//摄像机类方法-------------------------------------------------------------------------
Camera::Camera(Point pos,Point _screenCenter,double _zoom):
position(pos),screenCenter(_screenCenter),zoom(_zoom){

}

Point Camera::transWorldToScreen(int world_x,int world_y){
    int screen_center_x=this->screenCenter.getX();
    int screen_center_y=this->screenCenter.getY();
    int camera_x=this->position.getX();
    int camera_y=this->position.getY();
    int screen_x=(world_x - camera_x) * this->zoom + screen_center_x;
    int screen_y=(world_y - camera_y) * this->zoom + screen_center_y;
    return Point(screen_x,screen_y);
}

Point Camera::transScreenToWorld(int screen_x,int screen_y){
    int screen_center_x=this->screenCenter.getX();
    int screen_center_y=this->screenCenter.getY();
    int camera_x=this->position.getX();
    int camera_y=this->position.getY();
    int world_x=(screen_x - screen_center_x) / this->zoom + camera_x;
    int world_y=(screen_y - screen_center_y) / this->zoom + camera_y;
    return Point(world_x,world_y);
}

Camera::~Camera(){

}

//动画类方法---------------------------------------------------------------------------
Animation::Animation(){
    this->count=0;
    this->gameTimePerFrame=1;
    this->tick=1;
    this->parent=nullptr;
}

void Animation::playAnimation(){
    int size=this->AnimationFrameContainer.size();//动画的帧数
    this->tick++;
    if(this->count>=size){//超出最大帧
        this->count=0;
    }
    if(this->tick >= this->gameTimePerFrame){
        this->parent->reloadSimilarImage(this->AnimationFrameContainer[count]);//修改当前图片内容
        count++;//下一帧
        this->tick=0;
    }
    return;
}

Animation::~Animation(){

}
//双缓冲图像类方法------------------------------------------------------------------
Image::Image(PointF _leftUpPos,IMAGE* img,int Priority):
width(img->getwidth()),height(img->getheight()),priority(Priority){
    this->leftUpPos=new PointF;
    this->leftUpPos->setX(_leftUpPos.getX());
    this->leftUpPos->setY(_leftUpPos.getY());
    this->BuiltInAnimationContainer.resize(this->maxBuiltInAnimation,nullptr);
    Pixels.resize(this->height * this->width);
    DWORD* pixelData=GetImageBuffer(img);
    int size=this->height * this-> width;
    for(int i=0;i<size;i++){
        Pixel& pix=this->Pixels[i];
        uint32_t color=pixelData[i];
        pix.red=(color>>16) & 0xff;//r
        pix.green=(color>>8) & 0xff;//g
        pix.blue=color & 0xff;//b
    }
    return;
}
void Image::transRgbToBlackAndWhite(){
    for(Pixel& pix:this->Pixels){
        uint8_t Gray=0.299 * pix.red + 0.587 * pix.green + 0.114 * pix.blue;
        if(Gray<128){
            pix.red=0;
            pix.green=0;
            pix.blue=0;
        }
        else{
            pix.red=255;
            pix.green=255;
            pix.blue=255;
        }
    }
    return;
}
void Image::reloadSimilarImage(IMAGE* img){
    if(this->width != img->getwidth() || this->height != img->getheight()){
        return;
    }
    DWORD* pixelData=GetImageBuffer(img);
    int size=this->height * this->width;
    for(int i=0;i<size;i++){
        Pixel& pix=this->Pixels[i];
        uint32_t color=pixelData[i];
        pix.red=(color>>16) & 0xff;
        pix.green=(color>>8) & 0xff;
        pix.blue=color & 0xff;
    }
    return;
}

void Image::playCustomAnimation(int number){
    this->CustomAnimationContainer[number]->playAnimation();
    this->ResetBuiltInAnimation(-1);
    this->ResetCustomAnimation(number);
    return;
}

void Image::ResetCustomAnimation(int number){
    int size=this->CustomAnimationContainer.size();
    for(int i=0;i<size;i++){
        if(i != number && this->CustomAnimationContainer[i] != nullptr){
            this->CustomAnimationContainer[i]->ResetAnimation();
        }
    }
    return;
}

void Image::ResetBuiltInAnimation(int number){
    int size=this->BuiltInAnimationContainer.size();
    for(int i=0;i<size;i++){
        if(i != number && this->BuiltInAnimationContainer[i] != nullptr){
            this->BuiltInAnimationContainer[i]->ResetAnimation();
        }
    }
    return;
}

Image::~Image(){

}

//双缓冲图像管理类方法----------------------------------------------------------------------------------
ImageManager::ImageManager(int ScreenWidth,int ScreenHeight):
screenWidth(ScreenWidth),screenHeight(ScreenHeight){
    this->camera=nullptr;
    int size=this->screenHeight * this->screenWidth;
    this->frontBuffer.resize(size);
    this->backBuffer.resize(size);
    this->screen=new IMAGE(this->screenWidth,this->screenHeight);//创建屏幕画面对象
}

void ImageManager::updateBuffer(){//加载图片,更新缓冲区
    std::copy(this->backBuffer.begin(),this->backBuffer.end(),this->frontBuffer.begin());//后缓冲区复制到前缓冲区
    DWORD* PixData=GetImageBuffer(this->screen);//获取屏幕缓冲区
    for(int i=0;i<this->frontBuffer.size();i++){//加载图片
        uint32_t color =(this->frontBuffer[i].red << 16)|
        (this->frontBuffer[i].green << 8)|this->frontBuffer[i].blue;
        PixData[i]=color;
    }
    for(Image* img : this->ImageContainer){//开始绘制后缓冲区
        if(img->getExist()==false){continue;}//图像不存在，下一个
        if(this->isImageInScreen(img)==false){continue;}//图片不在窗口内
        int width=img->getWidth();
        int height=img->getHeight();
        Point leftUp_P=this->camera->transScreenToWorld(0,0);
        Point rightDown_P=this->camera->transScreenToWorld(this->screenWidth-1,this->screenHeight-1);
        int leftUp_X=leftUp_P.getX(),leftUp_Y=leftUp_P.getY();
        int rightDown_X=rightDown_P.getX(),rightDown_Y=rightDown_P.getY();
        int img_leftUp_X=img->getPos().getX(),img_leftUp_Y=img->getPos().getY();
        int img_rightDown_X=img_leftUp_X + img->getWidth() - 1;
        int img_rightDown_Y=img_leftUp_Y + img->getHeight() - 1;
        int start_x=std::max(leftUp_X,img_leftUp_X),start_y=std::max(leftUp_Y,img_leftUp_Y);
        int end_x=std::min(rightDown_X,img_rightDown_X),end_y=std::min(rightDown_Y,img_rightDown_Y);
        for(int y=start_y;y<=end_y;y++){
            for(int x=start_x;x<=end_x;x++){
                Point screen_p=this->camera->transWorldToScreen(x,y);
                int cur_y=screen_p.getY();
                int cur_x=screen_p.getX();
                int buffer_index=this->screenWidth * cur_y + cur_x;//缓冲区数组索引
                int pix_index=(y - img_leftUp_Y) * width + x - img_leftUp_X;//点数组索引
                Pixel pix=img->getPixel(pix_index);
                uint32_t color= (pix.red<<16)|(pix.green<<8)|pix.blue;
                if(color!=BLANKPIX){//不为空白像素
                    this->backBuffer[buffer_index]=pix;
                }
            }
        }
    }
    return;
}

void ImageManager::addCamera(Camera* _camera){
    this->camera=_camera;
    this->camera->setScreenCenter(this->screenWidth / 2,this->screenHeight / 2);
    return;
}

bool ImageManager::isImageInScreen(Image* img){
    Point leftUp_p=img->getPos();//世界坐标左上角
    int leftUp_x=leftUp_p.getX();
    int leftUp_y=leftUp_p.getY();
    int rightDown_x=leftUp_x + img->getWidth() -1;
    int rightDown_y=leftUp_y + img->getHeight() -1;
    Point LeftUp_P=this->camera->transWorldToScreen(leftUp_p);//坐标转化后左上角
    Point RightDown_P=this->camera->transWorldToScreen(rightDown_x,rightDown_y);
    if(LeftUp_P.getX() >= this->screenWidth || LeftUp_P.getY() >= this->screenHeight
    || RightDown_P.getX() < 0 || RightDown_P.getY() < 0){//不在窗口内
        return false;
    }
    return true;
}

bool ImageManager::isPointInScreen(Point screen_p){
    int screen_x=screen_p.getX();
    int screen_y=screen_p.getY();
    if(screen_x < 0 || screen_x >= this->screenWidth
    || screen_y < 0 || screen_y >= this->screenHeight){
        return false;
    }
    return true;
}

ImageManager::~ImageManager(){

}

//动态碰撞体类方法------------------------------------------------------------------------
DynamicCollisionObject::DynamicCollisionObject(PointF* _curPos,bool* _isExist,int _width,int _height,int _grid_width,int _grid_height):
curPos(_curPos),isExist(_isExist),width(_width),height(_height),grid_width(_grid_width),grid_height(_grid_height){
    this->prePos.setX(this->curPos->getX());
    this->prePos.setY(this->curPos->getY());
}

void DynamicCollisionObject::insertRectCollision(int x1,int y1,int x2,int y2){//矩形碰撞体
    Point startGrid=CollisionGrid(grid_width,grid_height).transPointToGrid(Point(x1,y1));
    Point endGrid=CollisionGrid(grid_width,grid_height).transPointToGrid(Point(x2,y2));
    Point centerGrid=CollisionGrid(grid_width,grid_height).transPointToGrid(Point((x1+x2)/2,(y1+y2)/2));
    int GridStart_x=startGrid.getX();
    int GridStart_y=startGrid.getY();
    int GridEnd_x=endGrid.getX();
    int GridEnd_y=endGrid.getY();
    int GridCenter_x=centerGrid.getX();
    int GridCenter_y=centerGrid.getY();
    if(GridStart_y==GridEnd_y){
        for(int x=GridStart_x;x<=GridEnd_x;x++){
            this->insertCheckPoint(x-GridCenter_x,GridStart_y-GridCenter_y);
        }
    }
    else{
        for(int x=GridStart_x;x<=GridEnd_x;x++){
            this->insertCheckPoint(x-GridCenter_x,GridStart_y-GridCenter_y);
            this->insertCheckPoint(x-GridCenter_x,GridEnd_y-GridCenter_y);
        }
        for(int y=GridStart_y+1;y<GridEnd_y;y++){
            this->insertCheckPoint(GridStart_x-GridCenter_x,y-GridCenter_y);
            this->insertCheckPoint(GridEnd_x-GridCenter_x,y-GridCenter_y);
        }  
    }
    return;
}

void DynamicCollisionObject::clearInside(std::vector<std::vector<bool>>& collisionArray){
    int row=collisionArray.size();
    int col=collisionArray[0].size();
    std::vector<Point> clearArray;
    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
            if(i-1>=0 && i+1<row && j-1>=0 && j+1<col && collisionArray[i][j]==true){//该检测点有上下左右
                bool up=collisionArray[i-1][j];
                bool down=collisionArray[i+1][j];
                bool left=collisionArray[i][j-1];
                bool right=collisionArray[i][j+1];
                if(up && down && left && right){
                    clearArray.push_back(Point(j,i));//加入删除数组
                }
            }
        }
    }
    for(Point p:clearArray){//拿出点
        collisionArray[p.getY()][p.getX()]=false;
    }
    return;
}

void DynamicCollisionObject::insertCircleCollision(int x,int y,int r){
    int width_r=r % this->grid_width==0?r / this->grid_width:r / this->grid_width + 1;
    int height_r=r % this->grid_height==0?r / this->grid_height:r / this->grid_height + 1;
    int radius=(width_r + height_r) / 2;//统一半径
    int width_size=2 * width_r + 1;
    int height_size=2 * height_r + 1;
    std::vector<std::vector<bool>> CircleMap(height_size,std::vector<bool>(width_size,false));
    for(int y=0;y<height_size;y++){
        for(int x=0;x<width_size;x++){
            int dist_x=x - width_r;
            int dist_y=y - height_r;
            int dist=dist_x * dist_x + dist_y * dist_y;
            if(dist<=radius * radius){
                CircleMap[y][x]=true;//圆内,标记为碰撞检测点
            }
        }
    }
    this->clearInside(CircleMap);
    Point cur_point=CollisionGrid(this->grid_width,this->grid_height).transPointToGrid(Point(x,y));
    int cur_x=cur_point.getX();
    int cur_y=cur_point.getY();
    Point p(this->width / 2,this->height / 2);
    Point GridCenterPoint=CollisionGrid(this->grid_width,this->grid_height).transPointToGrid(p);
    int GridCenter_X=GridCenterPoint.getX();
    int GridCenter_Y=GridCenterPoint.getY();
    for(int y=0;y<height_size;y++){
        for(int x=0;x<width_size;x++){
            if(CircleMap[y][x]==true){
                int Grid_X=x - width_r + cur_x - GridCenter_X;
                int Grid_Y=y - height_r + cur_y - GridCenter_Y;
                this->insertCheckPoint(Grid_X,Grid_Y);
            }
        }
    }
    return;
}

void DynamicCollisionObject::
insertCollisionFromArray(std::vector<std::vector<bool>>& CollisionArray,Point leftUp){//依据数组生成
    Point StartGrid=CollisionGrid(this->grid_width,this->grid_height).transPointToGrid(leftUp);
    int start_x=StartGrid.getX();
    int start_y=StartGrid.getY();
    int row=CollisionArray.size();
    int col=CollisionArray[0].size();
    int width_size=col / this->grid_width + 2;//+2防止越界
    int height_size=row / this->grid_height + 2;
    std::vector<std::vector<bool>> GridArry(height_size,std::vector<bool>(width_size,false));
    for(int y=0;y<row;y++){
        for(int x=0;x<col;x++){
            if(CollisionArray[y][x]==true){
                Point p=CollisionGrid(this->grid_width,this->grid_height).transPointToGrid(Point(x,y));
                int Grid_X=p.getX();
                int Grid_Y=p.getY();
                GridArry[Grid_Y][Grid_X]=true;
            }
        }
    }
    this->clearInside(GridArry);
    Point GridCenterPoint =
    CollisionGrid(this->grid_width,this->grid_height).transPointToGrid(this->width / 2,this->height / 2);
    int GridCenter_X=GridCenterPoint.getX();
    int GridCenter_Y=GridCenterPoint.getY();
    for(int y=0;y<height_size;y++){
        for(int x=0;x<width_size;x++){
            if(GridArry[y][x]==true){
                int Grid_X=x + start_x - GridCenter_X;
                int Grid_Y=y + start_y - GridCenter_Y;
                this->insertCheckPoint(Grid_X,Grid_Y);
            }
        }
    }
    return;
}

void DynamicCollisionObject::insertCollisionFromImage(Image* img,Point leftUp){
    int index_y,index_x;
    Point StartGrid=CollisionGrid(this->grid_width,this->grid_height).transPointToGrid(leftUp);
    int start_x=StartGrid.getX(),start_y=StartGrid.getY();//从相对碰撞图片的leftUp位置开始生成碰撞体
    int width=img->getWidth();
    int height=img->getHeight();
    int size_x=width / this->grid_width + 2;//+2防止越界
    int size_y=height / this->grid_height + 2;
    std::vector<std::vector<bool>> GridArray(size_y,std::vector<bool>(size_x,false));//网格数组
    const std::vector<Pixel>& pixelArray=img->getImageData();
    int size=pixelArray.size();
    for(int i=0;i<size;i++){
        Pixel pix=pixelArray[i];
        uint32_t color = (pix.red<<16) | (pix.green<<8) | pix.blue;
        if(color != BLANKPIX){
            index_y=i / width;
            index_x=i % width;
            Point Grid_p=CollisionGrid(this->grid_width,this->grid_height).transPointToGrid(index_x,index_y);
            int Grid_X=Grid_p.getX();
            int Grid_Y=Grid_p.getY();
            GridArray[Grid_Y][Grid_X]=true;
        }
    }
    this->clearInside(GridArray);
    int center_x=this->width / 2, center_y=this->height / 2;
    Point GridCenter_p=CollisionGrid(this->grid_width,this->grid_height).transPointToGrid(center_x,center_y);
    int GridCenter_X=GridCenter_p.getX(),GridCenter_Y=GridCenter_p.getY();
    for(int y=0;y<size_y;y++){
        for(int x=0;x<size_x;x++){
            if(GridArray[y][x]==true){
                int Grid_X=start_x + x - GridCenter_X;
                int Grid_Y=start_y + y - GridCenter_Y;
                this->insertCheckPoint(Grid_X,Grid_Y);
            }
        }
    }
    return;
}


DynamicCollisionObject::~DynamicCollisionObject(){

}

//碰撞网格类方法--------------------------------------------------------------------------------------
CollisionGrid::CollisionGrid(int _gridWidth,int _gridHeight,int _worldWidth,int _worldHeight):
gridWidth(_gridWidth),gridHeight(_gridHeight),worldWidth(_worldWidth),
worldHeight(_worldHeight){
    if(this->worldWidth % this->gridWidth==0){
        this->gridColumns=this->worldWidth / this->gridWidth;
    }
    else{
        this->gridColumns=this->worldWidth / this->gridWidth + 1;
    }
    if(this->worldHeight % this->gridHeight==0){
        this->gridRows=this->worldHeight / this->gridHeight;
    }
    else{
        this->gridRows=this->worldHeight / this->gridHeight + 1;
    }
    int size=this->gridRows * this->gridColumns;
    this->GridContainer.resize(size);//设置网格总数
}

void CollisionGrid::insertSolidCollisionPoint(int x,int y){
    Point Grid=this->transPointToGrid(x,y);
    int Grid_X=Grid.getX();
    int Grid_Y=Grid.getY();
    int GridIndex=Grid_Y * this->gridColumns + Grid_X;//计算索引
    this->GridContainer[GridIndex].isSolid=true;
    return;
}

void CollisionGrid::insertSolidCollisionObject(Point LeftUpPos,std::vector<std::vector<bool>>&PixData){
     int start_x=LeftUpPos.getX();//左上角X
     int start_y=LeftUpPos.getY();//左上角Y
     int PixDataHeight=PixData.size();
     int PixDataWidth=PixData[0].size();
     for(int y=0;y<PixDataHeight;y++){
        for(int x=0;x<PixDataWidth;x++){
            if(PixData[y][x]==true){
                int cur_x=start_x + x;
                int cur_y=start_y + y;
                this->insertSolidCollisionPoint(cur_x,cur_y);
            }
        }
     }
     return;
}

void CollisionGrid::insertSolidCollisionFromImage(Image* img,int leftUp_x,int leftUp_y){
    std::vector<Pixel> pixData=img->getImageData();
    int img_width=img->getWidth();
    int img_height=img->getHeight();
    for(int y=0;y<img_height;y++){
        for(int x=0;x<img_width;x++){
            int pix_index=y * img_width + x;
            uint8_t red=pixData[pix_index].red;
            uint8_t green=pixData[pix_index].green;
            uint8_t blue=pixData[pix_index].blue;
            uint32_t color = (red<<16) | (green<<8) | blue;
            if(color != BLANKPIX){
                this->insertSolidCollisionPoint(leftUp_x + x,leftUp_y + y);
            }
        }
    }
    return;
}

void CollisionGrid::insertRectSolidCollision(int x1,int y1,int x2,int y2){
    for(int y=y1;y<=y2;y++){
        for(int x=x1;x<=x2;x++){
            this->insertSolidCollisionPoint(x,y);
        }
    }
    return;
}

void CollisionGrid::insertCircleSolidCollision(int x,int y,int r){
    int start_x=x - r;
    int start_y=y - r;
    int end_x=x + r;
    int end_y=y + r;
    for(int cur_y=start_y;y<=end_y;y++){
        for(int cur_x=start_x;x<=end_x;x++){
            int dist_x=(cur_x - x) * (cur_x - x);
            int dist_y=(cur_y - y) * (cur_y - y);
            if(dist_x + dist_y <= r * r){//圆内
                this->insertSolidCollisionPoint(cur_x,cur_y);
            }
        }
    }
    return;
}

void CollisionGrid::SolidCollisionDetection(){
    for(DynamicCollisionObject* dynamicObject:this->DynamicCollisionContainer){
        if(dynamicObject->getIsExist()==false){
            continue;
        }
        int x=(int)dynamicObject->getCurPos().getX();
        int y=(int)dynamicObject->getCurPos().getY();
        int center_x=x + dynamicObject->getWidth() / 2;//中心点X
        int center_y=y + dynamicObject->getHeight() / 2;//中心点Y
        int Grid_X,Grid_Y;
        Point Grid_P;
        Grid_P=this->transPointToGrid(center_x,center_y);
        Grid_X=Grid_P.getX();
        Grid_Y=Grid_P.getY();
        if(this->IsPosReachable(Grid_X,Grid_Y,dynamicObject)==false){//不可到达
            int cur_x=(int)dynamicObject->getCurPos().getX();
            int cur_y=(int)dynamicObject->getCurPos().getY();
            int pre_x=(int)dynamicObject->getPrePos().getX();
            int pre_y=(int)dynamicObject->getPrePos().getY();
            int add_x=dynamicObject->getWidth() / 2;
            int add_y=dynamicObject->getHeight() / 2;
            if(this->IsPosReachable(this->transPointToGrid(cur_x+add_x,pre_y+add_y),dynamicObject)==true){//只改y
                dynamicObject->getPrePos().setX(dynamicObject->getCurPos().getX());
                dynamicObject->getCurPos().setY(dynamicObject->getPrePos().getY());
            }
            else if(this->IsPosReachable(this->transPointToGrid(pre_x+add_x,cur_y+add_y),dynamicObject)==true){//只改x
                dynamicObject->getCurPos().setX(dynamicObject->getPrePos().getX());
                dynamicObject->getPrePos().setY(dynamicObject->getCurPos().getY());
            }
            else{//两个都改还不行
                dynamicObject->getCurPos().setX(dynamicObject->getPrePos().getX());
                dynamicObject->getCurPos().setY(dynamicObject->getPrePos().getY());
            }
        }
        else{
            dynamicObject->getPrePos().setX(dynamicObject->getCurPos().getX());
            dynamicObject->getPrePos().setY(dynamicObject->getCurPos().getY());
        }
    }
}

bool CollisionGrid::IsPosReachable(int Grid_X,int Grid_Y,DynamicCollisionObject* dynamicObject){
    bool ans=false;
    int x=Grid_X, y=Grid_Y;
    for(Point point : dynamicObject->getCollisionCheckPoint()){//获取碰撞检测点
        Grid_X =x + point.getX();
        Grid_Y =y + point.getY();
        int GridIndex = Grid_Y * this->gridColumns + Grid_X;
        ans |= this->GridContainer[GridIndex].isSolid;
    }
    return !ans;//取非
}

Point CollisionGrid::transPointToGrid(int x , int y){//点坐标转网格坐标
    int Grid_X,Grid_Y;
    Grid_X=x % this->gridWidth==0 ? x / this->gridWidth : x / this->gridWidth + 1;
    Grid_Y=y % this->gridHeight==0 ? y / this->gridHeight : y / this->gridHeight + 1;
    Point Grid(Grid_X,Grid_Y);
    return Grid;
}

bool CollisionGrid::isFall(DynamicCollisionObject* dynamicCollisionObject){
    int x=(int)dynamicCollisionObject->getCurPos().getX();
    int y=(int)dynamicCollisionObject->getCurPos().getY();
    int center_x=x + dynamicCollisionObject->getWidth() / 2;//中心点X
    int center_y=y + dynamicCollisionObject->getHeight() / 2;//中心点Y
    Point Grid_P=this->transPointToGrid(center_x,center_y);
    int Grid_X=Grid_P.getX();
    int Grid_Y=Grid_P.getY();
    bool ans=this->IsPosReachable(Grid_X,Grid_Y + 1,dynamicCollisionObject);//下方点可达
    return ans;
}

bool CollisionGrid::isPointSolid(int grid_x,int grid_y){
    int grid_index=grid_y * this->gridColumns + grid_x;
    return this->GridContainer[grid_index].isSolid;
}

CollisionGrid::~CollisionGrid(){

}

//NPC类方法-----------------------------------------------------------------------------------------
NPC::NPC(PointF _curPos,double _verticalSpeed,double _horizontalSpeed,CharacterData _characterData,int _unitTime):
curPos(_curPos),verticalSpeed(_verticalSpeed),horizontalSpeed(_horizontalSpeed),healthAndBuffManager(_characterData),unitTime(_unitTime),
physicsManager(&verticalSpeed,&horizontalSpeed,_unitTime),
moveManager(&curPos,&verticalSpeed,&horizontalSpeed,_unitTime){
    this->image=nullptr;
}

void NPC::updateAllStatus(){
    this->physicsManager.updatePhysicsStatus();
    this->moveManager.updateMoveStatus();
    return;
}

void NPC::addImage(IMAGE* img,int priority){
    this->image = new Image(PointF(0,0),img,priority);
    this->image->setPos(&this->curPos);
    return;
}
NPC::~NPC(){

}

//寻路类方法--------------------------------------------------------------------------------------
PathFinding::PathFinding(){

}
int PathFinding::Manhattan_mode=(1 << 0);
int PathFinding::Diagonal_mode=(1 << 1);
int PathFinding::Euclidean_mode=(1 << 2);

int PathFinding::Manhattan_dist(int begin_x,int begin_y,int end_x,int end_y){//曼哈顿距离
    return abs(end_x - begin_x) + abs(end_y - begin_y);
}

int PathFinding::Diagonal_dist(int begin_x,int begin_y,int end_x,int end_y){//对角线距离
    return std::max(abs(end_x - begin_x),abs(end_y - begin_y));
}

int PathFinding::Euclidean_dist(int begin_x,int begin_y,int end_x,int end_y){//欧氏距离
    int dist_x=abs(end_x - begin_x);
    int dist_y=abs(end_y - begin_y);
    return (int)sqrt(dist_x * dist_x + dist_y * dist_y);
}

std::vector<Point> PathFinding::
A_Star(DynamicCollisionObject* _object,Point _end,CollisionGrid& _map,int _mode){//A*寻路
    Point _begin=Point(_object->getCurPos().getX(),_object->getCurPos().getY());//图片左上角坐标
    Point _center_add=_map.transPointToGrid(_object->getWidth() / 2,_object->getHeight() / 2);//中心网格偏移量
    Point Grid_begin=_map.transPointToGrid(_begin);//图片左上角网格坐标
    Point Grid_end=_map.transPointToGrid(_end);//目标点网格坐标
    int begin_x=Grid_begin.getX();
    int begin_y=Grid_begin.getY();
    int end_x=Grid_end.getX();
    int end_y=Grid_end.getY();
    int row=abs(end_y - begin_y) + 1;
    int col=abs(end_x - begin_x) + 1;
    int start_x=begin_x < end_x ? 0 : begin_x - end_x;//相对开始坐标
    int start_y=begin_y < end_y ? 0 : begin_y - end_y;
    int finish_x=begin_x < end_x ? end_x - begin_x : 0;//相对结束坐标
    int finish_y=begin_y < end_y ? end_y - begin_y : 0;
    std::vector<std::vector<Point>> come_from(row,std::vector<Point>(col));
    std::vector<std::vector<int>> dist(row,std::vector<int>(col,0x7fffffff));
    std::vector<std::vector<bool>> visit(row,std::vector<bool>(col,false));
    dist[start_y][start_x]=0;
    std::priority_queue<Node> heap;
    int (PathFinding::* f)(int,int,int,int);
    if(_mode == Manhattan_mode){f=Manhattan_dist;}
    else if(_mode == Diagonal_mode){f=Diagonal_dist;}
    else if(_mode == Euclidean_mode){f=Euclidean_dist;}
    else{return std::vector<Point>();}
    heap.push({start_x,start_y,(this->*f)(start_x,start_y,finish_x,finish_y)});
    bool find_path=false;
    while(!heap.empty()){
        Node cur=heap.top();
        heap.pop();
        int cur_x=cur.x;
        int cur_y=cur.y;
        if(cur_x == finish_x && cur_y == finish_y){
            find_path=true;
            break;//找到路了
        }
        if(visit[cur_y][cur_x]== true){
            continue;//来过了
        }
        visit[cur_y][cur_x]=true;
        for(int i=0;i<4;i++){
            int next_x = cur_x + this->dir[i][0];
            int next_y = cur_y + this->dir[i][1];
            if(next_x < 0 || next_x >= col
            || next_y < 0 || next_y >= row
            || _map.IsPosReachable(next_x - start_x + begin_x + _center_add.getX()
            ,next_y - start_y + begin_y + _center_add.getY(),_object)==false){
                continue;//越界或有障碍
            }
            int next_dist=(this->*f)(next_x,next_y,finish_x,finish_y);
            if(visit[next_y][next_x]==false
            && dist[cur_y][cur_x] + 1 < dist[next_y][next_x]){
                come_from[next_y][next_x]=Point(cur_x,cur_y);
                dist[next_y][next_x]=dist[cur_y][cur_x] + 1;
                heap.push({next_x,next_y,next_dist + dist[next_y][next_x]});
            }
        }
    }
    std::vector<Point> tmp;
    if(find_path==false){
        return tmp;
    }
    int x=finish_x,y=finish_y;
    while(x != start_x || y != start_y){
        tmp.push_back(Point(x - start_x + begin_x,y - start_y + begin_y));
        Point pre_p=come_from[y][x];
        x=pre_p.getX();
        y=pre_p.getY();
    }
    tmp.push_back(Point(x - start_x + begin_x,y - start_y + begin_y));
    std::vector<Point> path(tmp.rbegin(),tmp.rend());
    return path;
}

PathFinding::~PathFinding(){

}
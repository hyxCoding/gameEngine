#ifndef _GAME_ENGINE_H
#define _GAME_ENGINE_H
#define UNITTIME 1//单位移动时间
#define UPDIR -1//向上方向
#define DOWNDIR 1//向下方向
#define LEFTDIR -1//向左方向
#define RIGHTDIR 1//向右方向
#define LEFT_KEY 'A'//左移键
#define RIGHT_KEY 'D'//右移键
#define JUMP_KEY 'K'//跳跃键
#define BLANKPIX 0xFFFFFF //rgb为纯白色表示这个像素为透明
#include <iostream>
#include <graphics.h>
#include <algorithm>
#include <vector>
#include <queue>
class Image;
class PlayerController;
class PhysicsManager;
class MoveManager;
class DynamicCollisionObject;
class PointF{
private:
    double x;
    double y;
public:
    double getX(){return x;}
    double getY(){return y;}
    void setX(double X){this->x=X;}
    void setY(double Y){this->y=Y;}
    PointF(double X=0,double Y=0){this->x=X;this->y=Y;}
    PointF operator+(PointF other){
        PointF new_PointF;
        new_PointF.setX(this->getX()+other.getX());
        new_PointF.setY(this->getY()+other.getY());
        return new_PointF;
    }
    PointF& operator+=(PointF other){
        this->x+=other.getX();
        this->y+=other.getY();
        return *this;
    }
    PointF& operator=(PointF& other){
        this->x=other.x;
        this->y=other.y;
        return *this;
    }
    PointF operator-(){//负号
        PointF new_PointF;
        new_PointF.setX(-this->x);
        new_PointF.setY(-this->y);
        return new_PointF;
    }
    PointF operator-(PointF other){//减号
        PointF tmp=other;
        return this->operator+(-tmp);
    }
    PointF operator-=(PointF other){
        this->x-=other.getX();
        this->y-=other.getY();
        return *this;
    }
    friend std::ostream& operator<<(std::ostream& os,PointF& other){
        os<<'('<<other.getX()<<','<<other.getY()<<')';
        return os;
    }
    friend std::ostream& operator<<(std::ostream& os,PointF&& other){
        os<<'('<<other.getX()<<','<<other.getY()<<')';
        return os;
    }
    ~PointF(){}
};

class Point{
private:
    int x;
    int y;
public:
    Point(int X=0,int Y=0):x(X),y(Y){}
    Point(PointF P):x((int)P.getX()),y((int)P.getY()){}
    void setX(int new_x){x=new_x;}
    void setY(int new_y){y=new_y;}
    int getX(){return x;}
    int getY(){return y;}
    friend std::ostream& operator<<(std::ostream& os,Point& other){
        os<<'('<<other.getX()<<','<<other.getY()<<')';
        return os;
    }
    friend std::ostream& operator<<(std::ostream& os,Point&& other){
        os<<'('<<other.getX()<<','<<other.getY()<<')';
        return os;
    }
    ~Point(){}
};


//摄像机类----------------------------------------------------------------------------------------
class Camera{
private:
    Point position;
    Point screenCenter;//屏幕中心
    double zoom;//摄像机放大倍率,1.0正常
    
public:
    Camera(Point pos,Point _screenCenter=Point(0,0),double _zoom=1.0);
    void setScreenCenter(int x,int y){
        screenCenter.setX(x);
        screenCenter.setY(y);
    }
    void setScreenCenter(Point pos){
        screenCenter.setX(pos.getX());
        screenCenter.setY(pos.getY());
    }
    void setPosition(int x,int y){
        position.setX(x);
        position.setY(y);
    }
    void setPosition(Point pos){
        setPosition(pos.getX(),pos.getY());
    }
    void setPosition(double x,double y){
        setPosition((int)x,(int)y);
    }
    void setPosition(PointF pos){
        setPosition(pos.getX(),pos.getY());
    }
    Point getPosition(){
        return position;
    }
    Point transScreenToWorld(int screen_x,int screen_y);
    Point transScreenToWorld(Point screen_pos){
        return transScreenToWorld(screen_pos.getX(),screen_pos.getY());
    }
    Point transWorldToScreen(int world_x,int world_y);
    Point transWorldToScreen(Point world_pos){
        return transWorldToScreen(world_pos.getX(),world_pos.getY());
    }
    ~Camera();
};

struct Pixel{//像素点-----------------------------------------------------------------------------------
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

//动画类---------------------------------------------------------------------------------------------------
class Animation{
private:
    int count;//当前播放到动画的第几帧
    Image* parent;//动画所作用在的图片的指针
    int gameTimePerFrame;//每这么多个游戏时间播放一帧动画
    int tick;//当前时刻
    std::vector<IMAGE*> AnimationFrameContainer;
public:
    Animation();
    void insertAnimationFrame(IMAGE* img){
        AnimationFrameContainer.push_back(img);
    }
    void ResetAnimation(){//重置动画
        count=0;
        tick=0;
    }
    void setGameTimePerFrame(int t){
        gameTimePerFrame=t;
    }
    void playAnimation();
    void setParent(Image* _parent){parent=_parent;}
    ~Animation();
};

//双缓冲图像类-----------------------------------------------------------------------------------------------
class Image{
private:
    PointF *leftUpPos;//左上角位置
    int width;//宽
    int height;//高
    std::vector<Pixel> Pixels;
    int priority;//绘图优先级,数字越大,优先级越大
    bool exist=true;//图片是否存在 
    std::vector<Animation*> CustomAnimationContainer;//自定义动画
    std::vector<Animation*> BuiltInAnimationContainer;//内置动画
    int maxBuiltInAnimation=11;//内置动画数
private://内置动画的编号
    int IDLE=    0;//待机
    int WALK=    1;//行走
    int RUN=     2;//跑步
    int SPRINT=  3;//冲刺
    int JUMP=    4;//跳跃
    int FALL=    5;//下落
    int LAND=    6;//落地
    int CROUCH=  7;//蹲下
    int ATTACK=  8;//攻击
    int HURT=    9;//受伤
    int DEATH=   10;//死亡
private:
    void ResetCustomAnimation(int number);//重置除number外其余编号自定义动画
    void ResetBuiltInAnimation(int number);//重置除number外其余编号内置动画
public://内置动画
    void insertIdleAnimation(Animation* animation){
        animation->setParent(this);
        BuiltInAnimationContainer[IDLE]=animation;}
    void playIdleAnimation(){
        if(BuiltInAnimationContainer[IDLE]==nullptr){return;}
        BuiltInAnimationContainer[IDLE]->playAnimation();
        ResetBuiltInAnimation(IDLE);
        ResetCustomAnimation(-1);
    }
    void insertWalkAnimation(Animation* animation){
        animation->setParent(this);
        BuiltInAnimationContainer[WALK]=animation;}
    void playWalkAnimation(){
        if(BuiltInAnimationContainer[WALK]==nullptr){return;}
        BuiltInAnimationContainer[WALK]->playAnimation();
        ResetBuiltInAnimation(WALK);
        ResetCustomAnimation(-1);
    }
    void insertRunAnimation(Animation* animation){
        animation->setParent(this);
        BuiltInAnimationContainer[RUN]=animation;}
    void playRunAnimation(){
        if(BuiltInAnimationContainer[RUN]==nullptr){return;}
        BuiltInAnimationContainer[RUN]->playAnimation();
        ResetBuiltInAnimation(RUN);
        ResetCustomAnimation(-1);
    }
    void insertSprintAnimation(Animation* animation){
        animation->setParent(this);
        BuiltInAnimationContainer[SPRINT]=animation;}
    void playSprintAnimation(){
        if(BuiltInAnimationContainer[SPRINT]==nullptr){return;}
        BuiltInAnimationContainer[SPRINT]->playAnimation();
        ResetBuiltInAnimation(SPRINT);
        ResetCustomAnimation(-1);
    }
    void insertJumpAnimation(Animation* animation){
        animation->setParent(this);
        BuiltInAnimationContainer[JUMP]=animation;}
    void playJumpAnimation(){
        if(BuiltInAnimationContainer[JUMP]==nullptr){return;}
        BuiltInAnimationContainer[JUMP]->playAnimation();
        ResetBuiltInAnimation(JUMP);
        ResetCustomAnimation(-1);
    }
    void insertFallAnimation(Animation* animation){
        animation->setParent(this);
        BuiltInAnimationContainer[FALL]=animation;}
    void playFallAnimation(){
        if(BuiltInAnimationContainer[FALL]==nullptr){return;}
        BuiltInAnimationContainer[FALL]->playAnimation();
        ResetBuiltInAnimation(FALL);
        ResetCustomAnimation(-1);
    }
    void insertLandAnimation(Animation* animation){
        animation->setParent(this);
        BuiltInAnimationContainer[LAND]=animation;}
    void playLandAnimation(){
        if(BuiltInAnimationContainer[LAND]==nullptr){return;}
        BuiltInAnimationContainer[LAND]->playAnimation();
        ResetBuiltInAnimation(LAND);
        ResetCustomAnimation(-1);
    }
    void insertCrouchAnimation(Animation* animation){
        animation->setParent(this);
        BuiltInAnimationContainer[CROUCH]=animation;}
    void playCrouchAnimation(){
        if(BuiltInAnimationContainer[CROUCH]==nullptr){return;}
        BuiltInAnimationContainer[CROUCH]->playAnimation();
        ResetBuiltInAnimation(CROUCH);
        ResetCustomAnimation(-1);
    }
    void insertAttackAnimation(Animation* animation){
        animation->setParent(this);
        BuiltInAnimationContainer[ATTACK]=animation;}
    void playAttackAnimation(){
        if(BuiltInAnimationContainer[ATTACK]==nullptr){return;}
        BuiltInAnimationContainer[ATTACK]->playAnimation();
        ResetBuiltInAnimation(ATTACK);
        ResetCustomAnimation(-1);
    }
    void insertHurtAnimation(Animation* animation){
        animation->setParent(this);
        BuiltInAnimationContainer[HURT]=animation;}
    void playHurtAnimation(){
        if(BuiltInAnimationContainer[HURT]==nullptr){return;}
        BuiltInAnimationContainer[HURT]->playAnimation();
        ResetBuiltInAnimation(HURT);
        ResetCustomAnimation(-1);
    }
    void insertDeathAnimation(Animation* animation){
        animation->setParent(this);
        BuiltInAnimationContainer[DEATH]=animation;}
    void playDeathAnimation(){
        if(BuiltInAnimationContainer[DEATH]==nullptr){return;}
        BuiltInAnimationContainer[DEATH]->playAnimation();
        ResetBuiltInAnimation(DEATH);
        ResetCustomAnimation(-1);
    }
public:
    Image(PointF _leftUpPos,IMAGE* img,int Priority);
    void reloadSimilarImage(IMAGE* img);//重新载入图片,不允许改变图片长，宽
    void setPriority(int Priority){priority=Priority;}
    void setExist(bool Exist){exist=Exist;}//设置图片存在状态
    void transRgbToBlackAndWhite();//把Rgb图片转成黑白图
    void insertCustomAnimation(Animation* animation){//插入自定义动画
        animation->setParent(this);
        CustomAnimationContainer.push_back(animation);
    }
    void playCustomAnimation(int number);//播放自定义动画,number为动画编号，从0开始
    void deleteImage(){//删除图片
        exist=false;
        priority=0;//优先级最低，避免覆盖存在的图片
        Pixels.clear();//释放内存
    }
    bool getExist(){return exist;}//返回存在状态
    int getPriority(){return priority;}
    int getWidth(){return width;}
    int getHeight(){return height;}
    void setPos(PointF* _pos){leftUpPos=_pos;}
    Point getPos(){return Point((int)leftUpPos->getX(),(int)leftUpPos->getY());}
    const std::vector<Pixel>& getImageData(){
        return Pixels;
    }
    Pixel getPixel(int index){
        Pixel pix{Pixels[index].red,Pixels[index].green,Pixels[index].blue};
        return pix;
    }
    ~Image();
};

//双缓冲图像管理类
class ImageManager{
private:
    IMAGE* screen;//屏幕图片
    int screenWidth;//游戏窗口宽度
    int screenHeight;//游戏窗口高度
    std::vector<Pixel>frontBuffer;//前图片缓冲区
    std::vector<Pixel>backBuffer;//后图片缓冲区
    std::vector<Image*> ImageContainer;//图片容器
    Camera* camera;//摄像机
public:
    ImageManager(int ScreenWidth,int ScreenHeight);
    bool isImageInScreen(Image* img);//判断图片是否落在窗口显示区域内
    bool isPointInScreen(Point screen_p);//点是否在窗口内
    void InsertImage(Image* img){ImageContainer.push_back(img);}//插入图片
    void updateBuffer();//把前缓冲区数据加载到图像,更新缓冲区
    void updateScreen(){putimage(0,0,screen);}//显示图片
    void addCamera(Camera* _camera);//加入摄像机
    void sortImage(){
        sort(ImageContainer.begin(),ImageContainer.end(),[](Image* a,Image* b){//优先级小在前
            return a->getPriority() < b->getPriority();
        });
    }
    ~ImageManager();
};

class Gravity{//重力类-----------------------------------------------------------------------------------
private:
    double gravityAccel;//重力加速度
    double maxRisingSpeed;//最大上升速度
    double maxFallingSpeed;//最大下落速度
    bool active=true;//是否启用重力
    bool reverse=false;//是否启用反转重力
    PhysicsManager* parent;
public:
    Gravity(double GravityAccel=1,double MaxRisingSpeed=-2,double MaxFallingSpeed=2);
    void updateMoveStatus();//竖直位置更新
    void openReverse(){reverse=true;}//开启重力反转
    void closeReverse(){reverse=false;}//关闭重力反转
    void openGravity(){active=true;}//开启重力
    void closeGravity(){active=false;}//关闭重力
    void setParent(PhysicsManager* _parent){parent=_parent;}
    double getGravityAccel()const{return gravityAccel;}
    double& getGravityAccel(){return gravityAccel;}
    double getMaxRisingSpeed()const{return maxRisingSpeed;}
    double& getMaxRisingSpeed(){return maxRisingSpeed;}
    double getMaxFallingSpeed()const{return maxFallingSpeed;}
    double& getMaxFallingSpeed(){return maxFallingSpeed;}
    bool getActive()const{return active;}
    bool& getActive(){return active;}
    bool getReverse()const{return reverse;}
    bool& getReverse(){return reverse;}
    ~Gravity();
};

class HorizontalFriction{//水平摩擦力类-----------------------------------------------------------------
private:
    double frictionAccel;//摩擦力加速度
    bool active=true;//是否启用水平摩擦力
    bool reverse=false;//是否反转水平摩擦力方向
    double minOpenSpeedLimit=0.01;//摩擦力最小启动范围
    PhysicsManager* parent;
public:
    HorizontalFriction(double FrictionAccel=0.1);
    void openHorizontalFriction(){active=true;}//开启水平摩擦
    void closeHorizontalFriction(){active=false;}//关闭水平摩擦
    void openReverse(){reverse=true;}//开启反转
    void closeReverse(){reverse=false;}//关闭反转
    void updateMoveStatus();//更新运动状态
    void setParent(PhysicsManager* _parent){parent=_parent;}
    double getFrictionAccel()const{return frictionAccel;}
    double& getFrictionAccel(){return frictionAccel;}
    double getMinOpenSpeedLimit()const{return minOpenSpeedLimit;}
    double& getMinOpenSpeedLimit(){return minOpenSpeedLimit;}
    bool getActive()const{return active;}
    bool& getActive(){return active;}
    bool getReverse()const{return reverse;}
    bool& getReverse(){return reverse;}
    ~HorizontalFriction();
};

class AirFriction{//空气阻力类---------------------------------------------------------------------------
private:
    double frictionAccel;//摩擦加速度
    bool active=true;//启用
    bool reverse=false;//反转
    double minOpenSpeedLimit=0.01;//最小启动范围
    PhysicsManager* parent;
public:
    AirFriction(double FrictionAccel=0.2);
    void openAirFriction(){active=true;}//开启空气阻力
    void closeAirFriction(){active=false;}//关闭空气阻力
    void openReverse(){reverse=true;}//开启反转
    void closeReverse(){reverse=false;}//关闭反转
    void updateMoveStatus();//更新运动状态
    void setParent(PhysicsManager* _parent){parent=_parent;}
    double getFrictionAccel()const{return frictionAccel;}
    double& getFrictionAccel(){return frictionAccel;}
    double getMinOpenSpeedLimit()const{return minOpenSpeedLimit;}
    double& getMinOpenSpeedLimit(){return minOpenSpeedLimit;}
    bool getActive()const{return active;}
    bool& getActive(){return active;}
    bool getReverse()const{return reverse;}
    bool& getReverse(){return reverse;}
    ~AirFriction();
};


class Walk{//行走类--------------------------------------------------------------------
private:
    MoveManager* parent;
    double maxWalkSpeed;//最大自主左移速度
    double walkAccel;//水平加速度
public:
    Walk(double _maxWalkSpeed=2.0,double _walkAccel=0.2);
    void updateMoveStatus();//水平位置更新
    void setMaxWalkSpeed(double speed){maxWalkSpeed=speed;}
    void setWalkAccel(double _walkAccel){walkAccel=_walkAccel;}
    void setParent(MoveManager* _parent){
        parent=_parent;
    }
    ~Walk();
};
class Jump{//跳跃类--------------------------------------------------------------------------------
private:
    MoveManager* parent;
    int jumpCnt;//统计累计跳跃次数
    double jumpSpeed;//跳跃增加的竖直速度
    int maxJumpCnt;//最大跳跃次数
    double maxJumpSpeed;//最大跳跃可控速度
public:
    Jump(int JumpCnt,int MaxJumpCnt,double JumpSpeed=-15,double MaxJumpSpeed=-10);
    void setMaxJumpCnt(int cnt){maxJumpCnt=cnt;}
    void setJumpSpeed(double speed){jumpSpeed=speed;}
    void setMaxJumpSpeed(double speed){maxJumpSpeed=speed;}
    void resetJumpCnt(){jumpCnt=0;}
    void setParent(MoveManager* _parent){parent=_parent;}
    void updateMoveStatus();//跳跃更新
    ~Jump();
};

class Idle{//挂机类------------------------------------------------------------------------------------
private:
    MoveManager* parent;
public:
    Idle();
    void setParent(MoveManager* _parent){parent=_parent;}
    void updateMoveStatus();
    ~Idle();
};

class Run{//奔跑类------------------------------------------------------------------------------------
private:
    MoveManager* parent;
    double runAccel;
    double maxRunSpeed;
public:
    Run(double _runAccel=0.4,double _maxRunSpeed=4.0);
    void setParent(MoveManager* _parent){parent=_parent;}
    void setRunAccel(double _runAccel){runAccel=_runAccel;}
    void setMaxRunSpeed(double _maxRunSpeed){maxRunSpeed=_maxRunSpeed;}
    void updateMoveStatus();
    ~Run();
};

class Sprint{//冲刺类----------------------------------------------------------------------------------
private:
    MoveManager* parent;
    double sprintAccel;
    double maxSprintSpeed;
public:
    Sprint(double _sprintAccel=0.6,double _maxSprintSpeed=6.0);
    void setParent(MoveManager* _parent){parent=_parent;}
    void setSprintAccel(double _sprintAccel){sprintAccel=_sprintAccel;}
    void setMaxSprintSpeed(double _maxSprintSpeed){maxSprintSpeed=_maxSprintSpeed;}
    void updateMoveStatus();
    ~Sprint();
};

class Fall{//下落类------------------------------------------------------------------------------------
private:
    MoveManager* parent;
public:
    Fall();
    void setParent(MoveManager* _parent){parent=_parent;}
    void updateMoveStatus();
    ~Fall();

};

class Land{//着地类------------------------------------------------------------------------------------
private:
    MoveManager* parent;
public:
    Land();
    void updateMoveStatus();
    void setParent(MoveManager* _parent){parent=_parent;}
    ~Land();
};

class Crouch{//下蹲类-----------------------------------------------------------------------------------
private:
    MoveManager* parent;
public:
    Crouch();
    void setParent(MoveManager* _parent){parent=_parent;}
    void updateMoveStatus();
    ~Crouch();
};

class Attack{//普通攻击类----------------------------------------------------------------------------------
private:
    MoveManager* parent;
public:
    Attack();
    void setParent(MoveManager* _parent){parent=_parent;}
    void updateMoveStatus();
    ~Attack();
};

class Hurt{//受伤类-------------------------------------------------------------------------------------
private:
    MoveManager* parent;
public:
    Hurt();
    void setParent(MoveManager* _parent){parent=_parent;}
    void updateMoveStatus();
    ~Hurt();
};

class Death{//阵亡类------------------------------------------------------------------------------------
private:
    MoveManager* parent;
public:
    Death();
    void setParent(MoveManager* _parent){parent=_parent;}
    void updateMoveStatus();
    ~Death();
};

class Move{//移动类-------------------------------------------------------------------------------------
private:
    MoveManager* parent;
public:
    Move();
    void updatePos();//更新位置
    void setParent(MoveManager* _parent){
        parent=_parent;
    }
    ~Move();
};
struct Move_Status{//运动状态--------------------------------------------------------------------
    bool isLeft=false;
    bool isRight=false; 
    bool isIdle=false;
    bool isWalk=false;
    bool isRun=false;
    bool isSprint=false;
    bool isJump=false;
    bool isFall=false;
    bool isLand=false;
    bool isCrouch=false;
    bool isAttack=false;
    bool isHurt=false;
    bool isDeath=false;
};
class MoveManager{//运动管理类-----------------------------------------------------------------------
private:
    Move_Status move_status;
    PointF* curPos;
    double* verticalSpeed;
    double* horizontalSpeed;
    int unitTime;
    bool horizontalReverse=false;
    Image* image;
    Jump* jump;
    Walk* walk;
    Move* move;
    Idle* idle;
    Run* run;
    Sprint* sprint;
    Fall* fall;
    Land* land;
    Crouch* crouch;
    Attack* attack;
    Hurt* hurt;
    Death* death;
public:
    void addJump(Jump* _jump){
        jump=_jump;
        jump->setParent(this);
    }
    void addJump(int _jumpCnt,int _maxJumpCnt,double _jumpSpeed=-15,double _maxJumpSpeed=-10){
        jump=new Jump(_jumpCnt,_maxJumpCnt,_jumpSpeed,_maxJumpSpeed);
        jump->setParent(this);
    }
    void addWalk(Walk* _walk){
        walk=_walk;
        walk->setParent(this);
    }
    void addWalk(double _maxWalkSpeed=2.0,double _walkAccel=0.2){
        walk=new Walk(_maxWalkSpeed,_walkAccel);
        walk->setParent(this);
    }
    void addMove(Move* _move){
        move=_move;
        move->setParent(this);
    }
    void addMove(){
        move=new Move;
        move->setParent(this);
    }
    void addIdle(Idle* _idle){
        idle=_idle;
        idle->setParent(this);
    }
    void addIdle(){
        idle=new Idle;
        idle->setParent(this);
    }
    void addRun(Run* _run){
        run=_run;
        run->setParent(this);
    }
    void addRun(double _runAccel=4.0,double _maxRunSpeed=0.4){
        run=new Run(_runAccel,_maxRunSpeed);
        run->setParent(this);
    }
    void addSprint(Sprint* _sprint){
        sprint=_sprint;
        sprint->setParent(this);
    }
    void addSprint(double _sprintAccel=0.6,double _maxSprintSpeed=6.0){
        sprint=new Sprint(_sprintAccel,_maxSprintSpeed);
        sprint->setParent(this);
    }
    void addFall(Fall* _fall){
        fall=_fall;
        fall->setParent(this);
    }
    void addFall(){
        fall=new Fall;
        fall->setParent(this);
    }
    void addLand(Land* _land){
        land=_land;
        land->setParent(this);
    }
    void addLand(){
        land=new Land;
        land->setParent(this);
    }
    void addCrouch(Crouch* _crouch){
        crouch=_crouch;
        crouch->setParent(this);
    }
    void addCrouch(){
        crouch=new Crouch;
        crouch->setParent(this);
    }
    void addAttack(Attack* _attack){
        attack=_attack;
        attack->setParent(this);
    }
    void addAttack(){
        attack=new Attack;
        attack->setParent(this);
    }
    void addHurt(Hurt* _hurt){
        hurt=_hurt;
        hurt->setParent(this);
    }
    void addHurt(){
        hurt=new Hurt;
        hurt->setParent(this);
    }
    void addDeath(Death* _death){
        death=_death;
        death->setParent(this);
    }
    void addDeath(){
        death=new Death;
        death->setParent(this);
    }
public:
    void setImage(Image* img){image=img;}
    Image* getImage(){return image;}
    bool isLand();
public:
    MoveManager(PointF* _curPos,double* _verticalSpeed,double* _horizontalSpeed,int _unitTime);
    double getVerticalSpeed()const{return *verticalSpeed;}
    double& getVerticalSpeed(){return *verticalSpeed;}
    double getHorizontalSpeed()const{return *horizontalSpeed;}
    double& getHorizontalSpeed(){return *horizontalSpeed;}
    PointF getCurPos()const{return *curPos;}
    PointF& getCurPos(){return *curPos;}
    int getUnitTime(){return unitTime;}
    Move_Status& getMove_Status(){return move_status;}
    void updateMoveStatus();//更新运动
    void openHorizontalReverse(){horizontalReverse=true;}
    void closeHorizontalReverse(){horizontalReverse=false;}
    bool getHorizontalReverse(){return horizontalReverse;}
    Jump* getJump(){return jump;}
    Walk* getWalk(){return walk;}
    Move* getMove(){return move;}
    Idle* getIdle(){return idle;}
    Run* getRun(){return run;}
    Sprint* getSprint(){return sprint;}
    Fall* getFall(){return fall;}
    Land* getLand(){return land;}
    Crouch* getCrouch(){return crouch;}
    Attack* getAttack(){return attack;}
    Hurt* getHurt(){return hurt;}
    Death* getDeath(){return death;}
    ~MoveManager();
};

//物理计算管理类----------------------------------------------------------------------------------------
class PhysicsManager{
private:
    double* verticalSpeed;
    double* horizontalSpeed;
    int unitTime;
    Gravity* gravity;
    AirFriction* airFriction;
    HorizontalFriction* horizontalFriction;
public:
    void addGravity(Gravity* _gravity){
        gravity=_gravity;
        gravity->setParent(this);
    }
    void addGravity(double _gravityAccel=1,double _maxRisingSpeed=-2,double _maxFallingSpeed=2){
        gravity=new Gravity(_gravityAccel,_maxRisingSpeed,_maxFallingSpeed);
        gravity->setParent(this);
    }
    void addHorizontalFriction(HorizontalFriction* _horizontalFriction){
        horizontalFriction = _horizontalFriction;
        horizontalFriction->setParent(this);
    }
    void addHorizontalFriction(double _frictionAccel=0.1){
        horizontalFriction=new HorizontalFriction(_frictionAccel);
        horizontalFriction->setParent(this);
    }
    void addAirFriction(AirFriction* _airFriction){
        airFriction=_airFriction;
        airFriction->setParent(this);
    }
    void addAirFriction(double _frictionAccel=0.2){
        airFriction=new AirFriction(_frictionAccel);
        airFriction->setParent(this);
    }
    Gravity* getGravity(){return gravity;}
    AirFriction* getAirFriction(){return airFriction;}
    HorizontalFriction* getHorizontalFriction(){return horizontalFriction;}
public:
    PhysicsManager(double* _verticalSpeed,double* _horizontalSpeed,int _unitTime=UNITTIME);
    double getVerticalSpeed()const{return *verticalSpeed;}
    double& getVerticalSpeed(){return *verticalSpeed;}
    double getHorizontalSpeed()const{return *horizontalSpeed;}
    double& getHorizontalSpeed(){return *horizontalSpeed;}
    int getUnitTime(){return unitTime;}
    void updatePhysicsStatus();//更新物理状态
    ~PhysicsManager();
};
struct CharacterData{//人物基本信息----------------------------------------------------------------------
    double health;
    double attack;
};
class HealthAndBuffManager{//人物血量和buff管理类----------------------------------------------------------
private:
    CharacterData characterData;
public:
    HealthAndBuffManager(CharacterData _characterData);
    CharacterData getCharacterData()const{return characterData;}
    CharacterData& getCharacterData(){return characterData;}
    ~HealthAndBuffManager();
};

class PlayerController{//玩家控制类-----------------------------------------------------------------------
private:
    bool previousKeys[256]={false};
    bool currentKeys[256]={false};
    PointF curPos;
    double verticalSpeed;
    double horizontalSpeed;
    bool isExist;
    int unitTime=UNITTIME;
    PhysicsManager physicsManager;
    MoveManager moveManager;
    HealthAndBuffManager healthAndBuffManager;
    Image* image;
    DynamicCollisionObject* dynamicCollisionObject;
public:
    MoveManager& getMoveManager(){return moveManager;}
    PhysicsManager& getPhysicsManager(){return physicsManager;}
    HealthAndBuffManager& getHealthAndBuffManager(){return healthAndBuffManager;}
    DynamicCollisionObject* getDynamicCollisionObject(){return dynamicCollisionObject;}
    void addDynamicCollisionObject(DynamicCollisionObject* _dynamicCollisionObject);
    void addDynamicCollisionObject(PointF* _curPos,bool* _isExist,int _width,int _height,int _grid_width,int _grid_height);
public:
    void setIsLeft(bool left){moveManager.getMove_Status().isLeft=left;}
    void setIsRight(bool right){moveManager.getMove_Status().isRight=right;}
    void setIsIdle(bool idle){moveManager.getMove_Status().isIdle=idle;}
    void setIsWalk(bool walk){moveManager.getMove_Status().isWalk=walk;}
    void setIsRun(bool run){moveManager.getMove_Status().isRun=run;}
    void setIsSprint(bool sprint){moveManager.getMove_Status().isSprint=sprint;}
    void setIsJump(bool jump){moveManager.getMove_Status().isJump=jump;}
    void setIsFall(bool fall){moveManager.getMove_Status().isFall=fall;}
    void setIsLand(bool land){moveManager.getMove_Status().isLand=land;}
    void setIsCrouch(bool crouch){moveManager.getMove_Status().isCrouch=crouch;}
    void setIsAttack(bool attack){moveManager.getMove_Status().isAttack=attack;}
    void setIsHurt(bool hurt){moveManager.getMove_Status().isHurt=hurt;}
    void setIsDeath(bool death){moveManager.getMove_Status().isDeath=death;}
    bool getIsLeft(){return moveManager.getMove_Status().isLeft;}
    bool getIsRight(){return moveManager.getMove_Status().isRight;}
    bool getIsIdle(){return moveManager.getMove_Status().isIdle;}
    bool getIsWalk(){return moveManager.getMove_Status().isWalk;}
    bool getIsRun(){return moveManager.getMove_Status().isRun;}
    bool getIsSprint(){return moveManager.getMove_Status().isSprint;}
    bool getIsJump(){return moveManager.getMove_Status().isJump;}
    bool getIsFall(){return moveManager.getMove_Status().isFall;}
    bool getIsLand(){return moveManager.getMove_Status().isLand;}
    bool getIsCrouch(){return moveManager.getMove_Status().isCrouch;}
    bool getIsAttack(){return moveManager.getMove_Status().isAttack;}
    bool getIsHurt(){return moveManager.getMove_Status().isHurt;}
    bool getIsDeath(){return moveManager.getMove_Status().isDeath;}
public:
    PlayerController(PointF _curPos,double _verticalSpeed,double _horizontalSpeed,CharacterData _characterData);
    void updateKeyStatus(){
        for(int i=0;i<256;i++){
            previousKeys[i]=currentKeys[i];//覆盖
            currentKeys[i]=GetAsyncKeyState(i)&0x8000;//更新
        }
    }
    void updateAllStatus();//更新所有状态
    void addImage(IMAGE* img,int _priority);//加入图片
    void addImage(Image* img){
        image=img;
        moveManager.setImage(image);
    }
    Image* getImage(){return image;}
    PointF getCurPos()const{return curPos;}
    PointF& getCurPos(){return curPos;}
    bool anyKeyDown(uint8_t number){
        return previousKeys[number]&&currentKeys[number];
    }
    bool anyKeyPress(uint8_t number){
        return !previousKeys[number]&&currentKeys[number];
    }
    bool getIsExist()const{return isExist;}
    bool& getIsExist(){return isExist;}
    ~PlayerController();
};
//网格单元----------------------------------------------------------------------------------------------
struct GridCell{
    bool isSolid=false;//默认网格单元为空，即人物可以经过
};
class DynamicCollisionObject{//动态碰撞体类
private:
    PointF* curPos;
    PointF prePos;
    bool* isExist;
    int width;
    int height;
    int grid_width;
    int grid_height;
private:
    std::vector<Point> DynamicCollisionCheckPointContainer;//动态碰撞网格检测,x,y为相对中心点的偏移量
    void clearInside(std::vector<std::vector<bool>>& collisionArray);//清除内部检测点,保留边框
public:
    DynamicCollisionObject(PointF* _curPos,bool* _isExist,int _width,int _height,int _grid_width,int _grid_height);
    void insertCheckPoint(Point point){
        DynamicCollisionCheckPointContainer.push_back(point);
    }
    void insertCheckPoint(int x,int y){
        DynamicCollisionCheckPointContainer.push_back(Point(x,y));
    }
    void insertRectCollision(int x1,int y1,int x2,int y2);//加入矩形碰撞体
    void insertRectCollision(Point p1,Point p2){
        insertRectCollision(p1.getX(),p1.getY(),p2.getX(),p2.getY());
    }
    void insertCircleCollision(int x,int y,int r);//加入圆形碰撞体
    void insertCircleCollision(Point center,int radius){
        insertCircleCollision(center.getX(),center.getY(),radius);
    }
    //依据布尔数组生成碰撞体,true为检测点,leftUp为相对图片左上角的位置
    void insertCollisionFromArray(std::vector<std::vector<bool>>& CollisionArray,Point leftUp=Point(0,0));
    void insertCollisionFromImage(Image* img,Point leftUp=Point(0,0));//根据图片生成碰撞体
    void clearAllCollisionCheckPoint(){//清除所有碰撞检测点
        DynamicCollisionCheckPointContainer.clear();
    }
    std::vector<Point>& getCollisionCheckPoint(){
        return DynamicCollisionCheckPointContainer;
    }
    bool getIsExist(){return *isExist;}
    PointF getPrePos()const{return prePos;}
    PointF& getPrePos(){return prePos;}
    PointF getCurPos()const{return *curPos;}
    PointF& getCurPos(){return *curPos;}
    int getWidth(){return width;}
    int getHeight(){return height;}
    ~DynamicCollisionObject();
};
class CollisionGrid{//碰撞网格
private:
    int gridRows;//网格行数
    int gridColumns;//网格列数
    int gridWidth;//一个网格的宽度
    int gridHeight;//一个网格的高度
    int worldWidth;//世界宽度
    int worldHeight;//世界高度
    std::vector<GridCell> GridContainer;//网格容器
    std::vector<DynamicCollisionObject*> DynamicCollisionContainer;//动态碰撞体
public:
    bool isFall(DynamicCollisionObject* dynamicCollisionObject);
public:
    CollisionGrid(int width,int height){
        gridWidth=width;
        gridHeight=height;
    }
    CollisionGrid(int _gridWidth,int _gridHeight,int _worldWidth,int _worldHeight);
    void insertSolidCollisionPoint(int x,int y);//插件固定碰撞点
    void insertSolidCollisionPoint(Point pos){
        insertSolidCollisionPoint(pos.getX(),pos.getY());
    }
    void insertSolidCollisionObject(Point LeftUpPos,std::vector<std::vector<bool>>&PixData);//插入固定碰撞体
    void insertSolidCollisionObject(PointF LeftUpPos,std::vector<std::vector<bool>>&PixData){
        Point pos = Point((int)LeftUpPos.getX(),(int)LeftUpPos.getY());
        insertSolidCollisionObject(pos,PixData);
    }
    void insertSolidCollisionFromImage(Image* img,int leftUp_x,int leftUp_y);//按图片插入固定碰撞体
    void insertSolidCollisionFromImage(Image* img,Point LeftUpPos){
        insertSolidCollisionFromImage(img,LeftUpPos.getX(),LeftUpPos.getY());
    }
    void insertRectSolidCollision(int x1,int y1,int x2,int y2);//插入矩形固定碰撞体
    void insertRectSolidCollision(Point leftUp_p,Point rightDown_p){
        insertRectSolidCollision(leftUp_p.getX(),leftUp_p.getY(),rightDown_p.getX(),rightDown_p.getY());
    }
    void insertRectSolidCollision(Point P,int width,int height){
        insertRectSolidCollision(P.getX(),P.getY(),P.getX() + width - 1,P.getY() + height - 1);
    }
    void insertCircleSolidCollision(int x,int y,int r);//插入圆心固定碰撞体
    void insertCircleSolidCollision(Point P,int r){
        insertCircleSolidCollision(P.getX(),P.getY(),r);
    }
    void insertDynamicCollisionObject(DynamicCollisionObject* dynamicObject){//插入动态碰撞体
        DynamicCollisionContainer.push_back(dynamicObject);
    }
    void SolidCollisionDetection();//墙体碰撞检测
    Point transPointToGrid(int x,int y);//把点坐标转化成网格坐标
    Point transPointToGrid(double x,double y){
        return transPointToGrid((int)x , (int)y);
    }
    Point transPointToGrid(Point point){
        return transPointToGrid(point.getX() , point.getY());
    }
    Point transPointToGrid(PointF pointf){
        return transPointToGrid((int)pointf.getX() , (int)pointf.getY());
    }
    bool isPointSolid(int grid_x,int grid_y);
    bool isPointSolid(Point p){
        return isPointSolid(p.getX(),p.getY());
    }
    ~CollisionGrid();
public:
    bool IsPosReachable(int Grid_X,int Grid_Y,DynamicCollisionObject* dynamicObject);
    bool IsPosReachable(Point Grid,DynamicCollisionObject* dynamicObject){
        return IsPosReachable(Grid.getX(),Grid.getY(),dynamicObject);
    }
};

class NPC{//npc类--------------------------------------------------------------------------------
protected:
    PointF curPos;
    double verticalSpeed;
    double horizontalSpeed;
    int unitTime;
    PhysicsManager physicsManager;
    MoveManager moveManager;
    HealthAndBuffManager healthAndBuffManager;
    DynamicCollisionObject* dynamicCollisionObject;
    Image* image;
public:
    NPC(PointF _curPos,double _verticalSpeed,double _horizontalSpeed,
        CharacterData _characterData,int _unitTime=UNITTIME);
    PhysicsManager& getPhysicsManager(){return physicsManager;}
    MoveManager& getMoveManager(){return moveManager;}
    HealthAndBuffManager& getHealthAndBuffManager(){return healthAndBuffManager;}
    Image* getImage(){return image;}
    PointF getCurPos()const{return curPos;}
    PointF* getCurPos(){return &curPos;}
    void updateAllStatus();
    void addImage(IMAGE* img,int priority);
    void addImage(Image* img){image=img;}
    void addDynamicCollisionObject(DynamicCollisionObject* _dynamicCollisionObject){
        dynamicCollisionObject=_dynamicCollisionObject;
    }
    void addDynamicCollisionObject(PointF* _curPos,bool* _isExist,int _width,int _height,int _grid_width,int _grid_height){
        dynamicCollisionObject=new DynamicCollisionObject(_curPos,_isExist,_width,_height,_grid_width,_grid_height);
    }
    ~NPC();
};

class PathFinding{//寻路类-------------------------------------------------------------------------
private:
    struct Node{
        int x;
        int y;
        int distance;
        bool operator<(const Node& other)const{
            return other.distance < distance;
        }
    };
    int dir[4][2]={-1,0,1,0,0,-1,0,1};
    int Manhattan_dist(int begin_x,int begin_y,int end_x,int end_y);
    int Diagonal_dist(int begin_x,int begin_y,int end_x,int end_y);
    int Euclidean_dist(int begin_x,int begin_y,int end_x,int end_y);
public:
    PathFinding();
    std::vector<Point> A_Star(DynamicCollisionObject* _object,Point _end,CollisionGrid& _map,int _mode=Manhattan_mode);//A*
    ~PathFinding();
public:
    static int Manhattan_mode;
    static int Diagonal_mode;
    static int Euclidean_mode;
};

#endif
# ChaosDungeon

----- 코드 컨벤션 -------

카멜 표기법 
int backGround; -> 멤버변수, 지역변수, 매개변수 -> 변수 이름에서

파스칼표기법
int BackGround; -> 멤버함수, 람다 함수, 지역 변수 등등 함수 이름에서 쓰자.

- 중괄호 짝은 줄바꿈 하여 꼭 맞춰주자.
ex) 
for(int i = 0; i < 12; ++i)
{

{

while(true)
{
}

if(true)
{
}

- if문 내 1줄만 있더라도, 무조건 중괄호로 덮어주자.
ex)
if(true)
   a = 1;    -----> X

if(true)
{
   a = 1;    ------> O
}


- TObjectPtr<T>
-> 멤버변수 선언은 되도록이면 원시포인터가 아닌 TObjectPtr<T>를 사용하자.

-Getter 이나 Setter 같은 매우 간단한 함수는 앞에 inline or FORCEINLINE 을 붙여주자.

-Getter 함수 앞에 const를 붙여주자.

-클래스 헤더 내에 함수 영역 먼저, 그 다음 변수 영역 순으로 선언하고, 접근 제한자는 public, protected, private 순서다.

- 멤버변수 및 멤버함수 위에 짧게 기능에 대한 주석을 달아주자.

- 오버라이딩 된 함수 위에 주석으로 조상 or 부모 (맨 처음 부모의  가상함수)의 클래스 이름을 달아주자.
ex) // ParentClass::Func()
    virtual void Func() override;

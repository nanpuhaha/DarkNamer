# About

Darknamer is Batch File Renamer for Windows.

(Only support Korean)

# ChangeLog

- 2018-12-26: Create repository and update the latest code from original post.

# Feedback

1) 첨부 사진과 같이폴더내 파일명 일괄 변경시에(폴더명으로 맞춘후 폴더별로 뒤 번호붙임) 하위 폴더가 있고 그 하위 폴더의 파일들이 목록상 중간에 끼어있게 되면 본 폴더의 파일들 숫자가 이어지지 않고처음부터 부여되는 버그가 있네요. 파일위치로 정렬 했을때 하위폴더 파일 몇몇이 중간에 섞여 정렬이 되네요.

![](https://g-cbox.pstatic.net/20181012_219/1539293306012TU9Ov_PNG/du.PNG?type=m250_375)

2) 파일 날짜나 시간을 파일명에 삽입하는 기능도 있으면 좋겠습니다. EXIF 정보까지 추가할 수 있으면 더 좋겠구요.
> 현재 이름 앞/뒤 추가 기능의 옵션으로 생성 또는 변경 일시를 추가할수는 있습니다. (날짜와 시간을 따로 하는 기능은 없습니다.)

3) 윈도우즈 새로 설치하면 무조건 DarkNamer 다운받아서 dn.exe로 이름 바꿔서 C:\Windows\System32\에 넣어놓고 Win+R로 항상 필요할 때마다 실행시키곤 했습니다.

4) 숫자붙이기 기능에 '스탭'이라는 기능 추가 가능할까요? 스탭숫자를 지정하면 그 숫자만큼 건너뛰면서 정해지는거죠. 
> 시작 : 1
> 스탭 : 3
> 출력 : 1 4 7 10

5) RS5에서 호환안되는건지 드래그시 마우스가 다른 곳에서 이동되어버리는 버그가 생겼습니다. 예전 RS3 때는, 반디집 드래그시 다른 곳에서 이동되는 버그랑 동일하네요. 

6) 혹시 이런 기능도 있나요? 사진 파일을 1589(3-1), 1589(3-2), 1590(3-1), 1591(3-1) 이런 식으로 정리했을 경우, 파일 이름을 일괄적으로 1씩 줄이려고 합니다. 1588(3-1), 1588(3-2) 이렇게 말이죠. 

7) 번호 지우기도 좀 넣어주시면 안될까요? 의미없는 번호순 파일이 있어서요.

8) 단축키로 다 할 수 있으면 좋겠습니다! 마우스로 기능을 누르거나, F10 + T + ↑/↓ 로 할 수 있지만, 단축키로 하면 훨씬 더 수월할 것 같습니다. 픽픽처럼 개인이 단축키를 지정할 수 있으면 더욱 좋고, 픽픽에서 아쉬운 점이었던 '설정값을 백업하고 복원하는 기능'도 같이 추가된다면 정말 완벽할 것 같습니다.




# Credit

DarkNamer was orignally developed by [darkwalker](https://blog.naver.com/darkwalk77)


# Original Posts

### 1) [DarkNamer 08.02.10 소스코드](https://blog.naver.com/darkwalk77/70027450174)

2008. 2. 11. 0:37
by darkwalker

목록 정렬 버그 수정한 소스코드 입니다. (2018.8월 업데이트)

-----------------------------------------
Visual C++ 6.0 환경에서 MFC기반으로 제작.
생각보다 이것저것 사소한게 많이 걸려서 제작시간이 꽤나 길어졌습니다.
 
무엇보다도 수천개의 파일 로딩시 리소스 깨지는 문제 수정하느라 엄청 고생했군요.
 
공개된 소스코드는 자유롭게 참고하셔도 되지만 
그대로 가져가셔서 100% 무단 전제 및 활용하시는 것만은 좀 곤란합니다.


### 2) [파일 이름 일괄 변경 DarkNamer 08.02.10 실행파일 (목록 정렬 버그 수정)](https://blog.naver.com/darkwalk77/70027450806)

2008. 2. 11. 0:58
by darkwalker

안녕하세요, 실로 오랜만에 글을 올립니다. 벌써 마지막으로 업로드한지 10년이 지났네요.
몇가지 버그가 있다는 분들이 계셔서 확인해 보니
목록정렬 기능이 오작동하는 중요한 버그가 있었습니다.
그럼에도 불구하고 아직 많이 사용해 주시는 것 같아 감사합니다.
우선 해당 버그만 수정하여 새로 업데이트 하였습니다. 

---------------------------------------------------------

안녕하세요, Darknamer 개발자입니다. 
그간 업데이트 및 수정 요청주신 분들께 답변 드리지 못해 죄송합니다.
개인적인 사정으로 프로그래밍 작업이 어려운 상황이라
향후 공식 업데이트는 언제가 될지 말씀드리기 어려운 점 양해 부탁드립니다.
 
그간 과분한 호응 정말 감사드리며 
다른 게시물에 올라가 있는 소스코드는 자유롭게 가져다가 수정해서 사용하셔도 됩니다.
 
 
[파일 다운로드](http://mfiles.naver.net/158009baaef3f12d02e383bf8b6b1069c8946789f9/data33/2008/2/17/198/darknamer-darkwalk77.zip)

![](https://blogfiles.pstatic.net/data28/2008/2/11/295/dddd_darkwalk77.jpg)

 
파일 이름을 한꺼번에 바꿔주는 프로그램입니다. 예전부터 공개되어 있던 프로그램으로 
이전 버전과 비교해서 추가 기능은 파일 크기, 수정시각 등의 정보로 목록을 정렬하는 것 뿐이지만 
이것저것 수정을 거치는 과정에서 어쩌다 보니 대대적인 내부 수리를 했습니다.
수천개 이상의 파일 처리시 기능은 정상작동하지만 화면이 깨지는 버그가 수정되었습니다.
 
설치등은 전혀 필요 없고 실행파일을 아무곳에나 카피해놓고 바로 실행하시면 됩니다.
사용하실때는 기본적으로는 마우스로 파일을 끌어다가 놓으신 후에 이름을 바꾸고 
'변경 적용' 버튼을 누르면 파일명이 바뀝니다. 

### 3) [DarkNamer 18.08.30 베타버전 업데이트](https://blog.naver.com/darkwalk77/221349401402)

2018. 8. 30. 23:14
by darkwalker

안녕하세요, 실로 오랜만에 새로운 버전을 올립니다.

- 목록 정렬 기능 버그 수정 
- 숫자 남기기/삭제 기능 추가

아직 테스트가 끝나지 않은 베타 버전입니다만 기능은 일단 다 구현하였습니다.
darknamer_code.zip 은 소스코드 파일입니다.
사용을 위해서는 darknamer.zip 만 받으시면 됩니다.

조만간 추가 테스트를 마치고 업데이트 하도록 하겠습니다.
감사합니다.

![](https://blogfiles.pstatic.net/MjAxODA4MzBfMjA2/MDAxNTM1NjM4NjM3Nzg5.-Y3TqX_HDo3fk5qVdZQXfnCLBs1dvsh4uqPL-Atqzekg.55fAq5M31CyhATmAC2LCzL59RV0lU6fKUhWndK5fOccg.JPEG.darkwalk77/sample.jpg)

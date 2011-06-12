#include "scrollbarUI.h"

#include <iostream>
#include <sstream>
#include <vector>

#include "buttonUI.h"
#include "textureUI.h"
#include "log.h"
#include "misc.h"

void scrollbarProcessClick(frame *f,int id)
{
  (reinterpret_cast<scrollbarUI*>(f->parent))->clickReturn(id);
}

scrollbarUI::scrollbarUI(float xpos, float ypos, float h, int n)
{
	x=xpos;
	y=ypos;
	width=16;
	height=h;
	
	buttonUI* ScrollUp=new buttonUI(-6.0f,-8.0f,32.0f,32.0f,"Interface\\Buttons\\UI-ScrollBar-ScrollUpButton-Up.blp","Interface\\Buttons\\UI-ScrollBar-ScrollUpButton-Down.blp");
	ScrollUp->setClickFunc(scrollbarProcessClick,0);
	addChild(ScrollUp);

	buttonUI* ScrollDown=new buttonUI(-6.0f,height-24.0f,32.0f,32.0f,"Interface\\Buttons\\UI-ScrollBar-ScrollDownButton-Up.blp","Interface\\Buttons\\UI-ScrollBar-ScrollDownButton-Down.blp");
	ScrollDown->setClickFunc(scrollbarProcessClick,1);
	addChild(ScrollDown);
	ScrollKnob=new textureUI(-6.0f,10.0f,32.0f,32.0f,"Interface\\Buttons\\UI-ScrollBar-Knob.blp");
	addChild(ScrollKnob);
	value=0;
	num=n;
	changeFunc=0;
}

bool scrollbarUI::processLeftDrag(float /*mx*/,float my, float /*xChange*/, float /*yChange*/)
{
	if(num<0) false;
	float tx,ty;
	this->getOffset(&tx,&ty);
	my-=(ty+32);
	value=misc::FtoIround(num*my/(height-64));

	if(value<0)
		value=0;
	else if(value>num-1)
		value=num-1;

	if(num>0)
		setScrollNoob();

	if(changeFunc)
		changeFunc(this,value);
	return true;
}

frame *scrollbarUI::processLeftClick(float mx,float my)
{
	if(num<0) this;
	frame * lTemp;
	for( std::vector<frame*>::reverse_iterator child = children.rbegin(); child != children.rend(); child++ )
	{
		if( !( *child )->hidden && ( *child )->IsHit( mx, my ) )
		{
			lTemp = ( *child )->processLeftClick( mx - ( *child )->x, my - ( *child )->y );
			if( lTemp )
				return lTemp;
		}
	}
	return this;
}

void scrollbarUI::clickReturn(int id)
{
	if(num<0) return;
	if(id==0)//Scroll Up
	{
		if(value!=0)
			value--;
	}
	else if(id==1)//Scroll Down
	{
		if(value!=num-1)
			value++;
	}
	else//ScrollKnob
	{
		// handeld with processLeftClick and processLeftDrag
	}
	
	//Update ScrollKnob Position
	setScrollNoob();

	// call changeFunc if set
	if(changeFunc)
		changeFunc(this,value);
}

void scrollbarUI::setChangeFunc( void (*f)( frame *, int ))
{
	changeFunc=f;
}

int	 scrollbarUI::getValue() const
{
	return value;
}

void scrollbarUI::setValue(int i)
{
	if(i>-1 && i < num)
	value=i;
	setScrollNoob();
}

void scrollbarUI::setNum(int i)
{
	num=i;
	value=0;
	setScrollNoob();
}

void scrollbarUI::setScrollNoob( )
{
	if(num)
	{
		ScrollKnob->y=  11.0f + ( (this->height - 54.0f) / (num-1) ) *value;
	}
}

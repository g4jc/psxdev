// floating adverts.. copyright (C) 2000 by dbalster

function handler()
{
	if (document.move.top < 100)
	{
	document.move.top += 10;
	setTimeout("handler()",100);
	}
	else
	{
	setTimeout("handler2()",100);
	}
}

function handler2()
{
	if (document.move.left < 1000)
	{
	document.move.left += 1;
	setTimeout("handler2()",100);
	}
}

function install()
{
	setTimeout("handler()",100);
}

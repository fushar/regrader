function set_server_clock(serverTime)
{
	var curTime = new Date();
	var diffTime = serverTime - curTime;
	update_server_clock(diffTime);
}

function set_contest_clock(serverTime, endTime)
{
	var curTime = new Date();
	var diffTime = serverTime - curTime;
	update_contest_clock(diffTime, endTime);
}

function update_server_clock(diffTime)
{
	var curTime = new Date();
	curTime.setTime(curTime.getTime() + diffTime);

	var curHour = curTime.getHours();
	var curMinute = curTime.getMinutes();
	var curSecond = curTime.getSeconds();

	curMinute = (curMinute < 10 ? '0' : '') + curMinute;
	curSecond = (curSecond < 10 ? '0' : '' ) + curSecond;
	curHour = (curHour < 10 ? '0' : '') + curHour;

	var res = curHour + ':' + curMinute + ':' + curSecond;
	$('#server_clock').html(res);

	setTimeout(function(){update_server_clock(diffTime);}, 500);
}

function update_contest_clock(diffTime, endTime)
{
	var curTime = new Date();
	curTime.setTime(curTime.getTime() + diffTime);
	
	var res;
	if (curTime >= endTime)
		res = '00:00:00';
	else
	{
		var diff = endTime.getTime() - curTime.getTime();
		
		var curHour = Math.floor(diff / 3600000);
		diff %= 3600000;
		var curMinute = Math.floor(diff /  60000);
		diff %= 60000;
		var curSecond = Math.floor(diff / 1000);
		if (curHour < 10) curHour = '0' + curHour;
		if (curMinute < 10) curMinute = '0' + curMinute;
		if (curSecond < 10) curSecond = '0' + curSecond;
		res = curHour + ':' + curMinute + ':' + curSecond;
	}
	
	$('#contest_clock').html(res);
	setTimeout(function(){update_contest_clock(diffTime, endTime);}, 500);
}

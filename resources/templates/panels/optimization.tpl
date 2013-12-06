<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="generator" content="Agros2D" />
	<style type="text/css">
		{{STYLESHEET}}
	</style>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.js"></script>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.flot.js"></script>
    <script language="javascript" type="text/javascript" src="{{PANELS_DIRECTORY}}/js/jquery.flot.axislabels.js"></script>
</head>
<body>

<img style="float: right; margin-right: 10px; margin-top: 12px;" src="{{AGROS2D}}" /> 
<h1>{{NAME}}</h1>
<div>
<table class="heading">
</table>
</div>

<table>
<tr>
<td>
<div class="section">
<h2>XXX_nadpis1</h2>
<hr/>
number: {{POINT_NUMBER}}

</div>
</td>

<td>
<div class="section">
<h2>XXX_sloupec2</h2>
<hr/>


<div style="text-align: center; width: 400px; height: 400px;"> Optimization <br/>
<div id="chart_optimization" style="width: 100%; height: 90%;">
<script type="text/javascript">$(function ()
{
    $.plot($("#chart_optimization"), [
        {
          data: {{OPTIMIZATION_DATA}},
          color: "rgb(61, 61, 251)",
          lines: { show: false },
          points: { show: true }
        }
      ],
      {
        grid: { hoverable : true, clickable : true },
        xaxes: [ { axisLabel: 'func 1 (-)' } ],
        yaxes: [ { axisLabel: 'func 2 (-)' } ]
      });

    $("#chart_optimization").bind("plotclick", function (event, pos, item) {
                            if (item) {
                                    $("#clickdata").text(" - click point ");
                                    ehbridge.call(item.dataIndex);
                            }
                    });
});
</script>
</div>
</div>
text:
<span id="clickdata"></span>
konec
</div>
</td>
</tr>
</table>

<div class="section">
<h2>XXX_NadpisDole</h2>
<hr/>
{{#TRANSIENT_ADAPTIVE}}
<div style="text-align: center; width: 50%; height: 160px;">Time step length<br/><div id="chart_time_step_length" style="width: 100%; height: 90%;"></div></div>
{{TIME_STEPS_CHART}}
{{/TRANSIENT_ADAPTIVE}}
</div>
</td>

{{PROBLEM_DETAILS}}

<div class="cleaner"></div>

</body>
</html>


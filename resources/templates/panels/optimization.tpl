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
<h2>Design</h2>
<hr/>
<div class="figure">{{GEOMETRY_SVG}}</div></div>
<h2>Parameters</h2>
<hr/>
<table>
{{#PARAMETER_SECTION}}
        <tr><td><b>{{PARAMETER_LABEL}}:</b></td><td>{{PARAMETER_VALUE}}</td></tr>
{{/PARAMETER_SECTION}}
</table>

</td>

<td>
<div class="section">
<h2>Functionals</h2>
<hr/>


<div style="text-align: center; width: 400px; height: 400px;"> Pareto front <br/>
<div id="chart_optimization" style="width: 100%; height: 90%;">
<script type="text/javascript">$(function ()
{
    $.plot($("#chart_optimization"), [
        {
          data: {{OPTIMIZATION_DATA_NOT_FRONT}},
          name: "not_front",
          color: "rgb(61, 61, 251)",
          lines: { show: false },
          points: { show: true }
        },
        {
          data: {{OPTIMIZATION_DATA_FRONT}},
          name: "front",
          color: "rgb(251, 61, 61)",
          lines: { show: false },
          points: { show: true }
        }
      ],
      {
        grid: { hoverable : true, clickable : true },
        xaxes: [ { axisLabel: 'func 1 (-)',  min:{{XMIN}}, max:{{XMAX}} }],
        yaxes: [ { axisLabel: 'func 2 (-)', min:{{YMIN}}, max:{{YMAX}} } ]
      });

    $("#chart_optimization").bind("plotclick", function (event, pos, item) {
                            if (item) {
                                    $("#clickdata").text(" - click point ");
                                    ehbridge.call(item.dataIndex, item.series.name);
                            }
                    });
});
</script>
</div>
</div>

<table>
        <tr><td><b>{{FUNC1_LABEL}}:</b></td><td>{{FUNC1}}</td></tr>
        <tr><td><b>{{FUNC2_LABEL}}:</b></td><td>{{FUNC2}}</td></tr>
        <tr><td><b>&nbsp;</b></td><td>&nbsp;</td></tr>
        <tr><td><b>generation:</b></td><td>{{GENERATION}}</td></tr>
</table>

</div>
</td>
</tr>
</table>


{{PROBLEM_DETAILS}}

<div class="cleaner"></div>

</body>
</html>


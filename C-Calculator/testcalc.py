import subprocess, os

processname = './'+input()

tests = [
    ('10\n10\n6\n-1', '-1.000000'),
    ('10\n10\n6\n( -1)', '-1.000000'),
    ('10\n10\n4\n+1', '1.0000'),
    ('10\n10\n2\n(+1)', '1.00'),
    ('10\n2\n2\n+0.0', '0.00'),
    ('10\n10\n6\n( -1)', '-1.000000'),
    ('10\n10\n6\n2+3', '5.000000'),
    ('10\n10\n3\n2.00 +         3.000', '5.000'),
    ('10\n10\n2\n2.01 +       3.010', '5.02'),
    ('10\n10\n1\n2.11+3.12', '5.2'),
    ('10\n10\n1\n2.12*(-(-2.0))', '4.2'),
    ('10\n10\n1\n2.12*(-(  -2.0))/(-(-2.0))*(-(-2.0))/(-(-2.0))', '2.1'),
    ('2\n10\n5\n111.1-0.11*1.1', '6.37500'),
    ('9\n9\n6\n8.12/1.32-8.3*8-12.44*12.8', '-238.282557'),
    ('9\n10\n12\n8.12/1.32-8.3*8-12.44*12.8', '-197.324591594961'),
    ('9\n9\n1\n8.12/1.32-8.3*8-12.44*12.8', '-238.2'),
    ('3\n3\n8\n221.22/2.1-2.0/1.0*(+22.2)*(1.2-1.1)+2222+1111', '11122.02212010'),
    ('3\n10\n8\n-(221.22/2.1-2.0/1.0*(+22.2)*(1.2-1.1)+2222+1111)', '-125.31746031'),
    ('21\n10\n6\nA.B/B.A+9.C-GH.I/J.K+A.A-K.D', '-17.389499'),
    ('36\n18\n8\nZX-XC+AB-ZXC+DAS-FA*EW-QW', '-31BGG.00000000'),
    ('31\n23\n6\nET/UI+AA.R', 'DM.8628K7'),
    ('36\n10\n6\nM.ATAN-AS.M/RY.AKR*(-M.ATAN)+ZZZ-(AZ.Z-5.XY)', '46295.883035'),
    ('36\n21\n6\nA.Z/5.ZX+ZZ.XC-(   -XC+Y.Y)/0.Y+YDA', '51FC.74AGKA')
]

for testn in range(len(tests)):
    test, rightans = tests[testn]
    print('TEST', testn+1)
    print("INPUT:")
    print(test)
    spec = open('spec.txt', 'w')
    spec.write(test)
    spec.close()
    spec = open('spec.txt', 'r')
    try:
        sub_pipe = subprocess.Popen(processname, stdin=spec, stdout=subprocess.PIPE)
        output = sub_pipe.communicate(timeout=10)[0]
        gotans = (str(output)[2:-3]).split('\\n')[0]
        print('GOT:', gotans)
        print('EXPECTED:', rightans)
        if rightans != gotans:
            raise ValueError
        else:
            print('OK\n')
    except:
        print('ERROR\n')
        break
    spec.close()
    os.remove('spec.txt')
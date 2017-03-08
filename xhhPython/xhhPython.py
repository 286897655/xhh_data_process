import time
ens=[]
chargelist=[]
waitlist=[]
totaltime=totalnum=winddis=0

class en:
    def __init__(self,occur,types,ps):
        self.occur=occur
        self.types=types
        self.ps=ps
        
    @staticmethod
    def cmp(x):
        return x.occur
    
    @staticmethod
    def find(insid):
        for index,enum in enumerate(ens):
            if enum.ps==insid:
                return index
            


class battery:
    chargmin=[14,10,5,10,8,10,11,12,13,17]
    chargpow=[1.78354348,1.632628225,1.53659127,1.248480407,1.080415737,0.86433259,0.698450551,0.560215535,0.369372916,0.225969288]
    id=0
    
    def __init__(self,time,level,fill):
        self.starttime=time
        self.level=level
        self.fill=fill
        self.insid=battery.idadd()
        
    @staticmethod
    def cmp(x):
        return x.level
    
    @staticmethod
    def find(insid):
        for index,enum in enumerate(chargelist):
            if enum.insid==insid:
                return index
    
    @classmethod
    def idadd(cls):
        cls.id+=1
        return cls.id

class wind:
    f=open('./data.txt')
    prew=currw=time=wchange=0
    
    @classmethod
    def changeen(cls):
        while True:
            currwstr=cls.f.readline()
            cls.time+=10
            if not currwstr:
                break
            cls.currw=float(currwstr)*1000#make 1000 times than battery power.
            cls.wchange+=cls.currw-cls.prew    
            cls.prew=cls.currw
            if abs(cls.wchange)>0.5:#Sensitive, half battery!
                ens.append(en(cls.time,'wchange',cls.wchange))
                cls.wchange=0
                break
        

def charged(current):
    index=battery.find(current.ps)
    btry=chargelist[index]
    if btry.level<9:
        btry.level+=1
        btry.fill=0
        current.ps=battery.chargpow[btry.level-1]-battery.chargpow[btry.level]
        ens.append(en(current.occur+battery.chargmin[btry.level],'charged',btry.insid))
        powerchange(current) 
    elif btry.level==9:
        chargelist.pop(index)
        global totaltime,totalnum
        totalnum+=1
        totaltime+=current.occur-btry.starttime   
        current.ps=battery.chargpow[btry.level]
        powerchange(current)


def powerchange(current):
    changepow=current.ps
    global winddis
    changepow+=winddis
    if changepow>0:
        waitlist.sort(key=battery.cmp,reverse=True)#From small to big to turn on?
        #print(current.occur,changepow,len(chargelist))#hkjhkhkj..............................
        for index,btry in enumerate(waitlist):
            pow=battery.chargpow[btry.level]
            if pow<changepow:
                changepow-=pow
                waitlist.pop(index)
                chargelist.append(btry)
                ens.append(en(current.occur+battery.chargmin[btry.level]*(1-btry.fill/100),'charged',btry.insid))
            else:
                break
        while changepow>=battery.chargpow[0]:
            changepow-=battery.chargpow[0]
            btrytemp=battery(current.occur,0,0)
            chargelist.append(btrytemp)
            ens.append(en(current.occur+battery.chargmin[0],'charged',btrytemp.insid))   
        winddis=changepow
    if changepow<0:
        chargelist.sort(key=battery.cmp)#From big to small to turn off?
        for index,btry in enumerate(chargelist):
            pow=battery.chargpow[btry.level]
            if changepow<0:
                changepow+=pow
                chargelist.pop(index)
                waitlist.append(btry)
                tempen=ens.pop(en.find(btry.insid))
                btry.fill=100-(tempen.occur-current.occur)/battery.chargmin[btry.level]*100
        winddis=changepow

if __name__=='__main__':
    wind.changeen()
    #ens.append(en(1,'wchange',21*0.5))
    #ens.append(en(800,'wchange',-10*0.5))
    timepre=time.time()
    while ens:
        ens.sort(key=en.cmp)
        tempen=ens.pop(0)
        if tempen.occur>60*4:
            break
        if tempen.types=='wchange':
            powerchange(tempen)
            wind.changeen()
        elif tempen.types=='charged':
            charged(tempen)
            
    print(time.time()-timepre,totaltime/totalnum,len(chargelist))

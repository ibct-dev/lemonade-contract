import { Blockchain } from "@ibct-dev/sclest";
import { expect } from "chai";

function seconds_since_epoch(d: number) {
    return Math.floor(d / 1000);
}

beforeEach(async () => {
    await new Promise(resolve => setTimeout(resolve, 1000));
    console.log("----------------------");
});

describe("lemonade.c 컨트랙트 테스트", () => {
    const debug = 1;
    const isLocal = true;
    const bc = new Blockchain({ host: "127.0.0.1", port: "8888" }); // test

    /* 테스트 변수들 */
    const manager = "lemonade.c"; // 컨트랙트 배포 계정
    let contractTester: any;
    let errorCount = 0;
    const maximumSupply = 10000000;

    describe("테스트가 시작되면, 블록체인이 연결 되어야 함", async () => {
        it("블록체인을 연결함", async () => {
            expect(await bc.checkConntection()).eq(true);
        });
    });

    if (isLocal) {
        describe("로컬블록체인이라면, 컨트랙트를 새로 배포해야 함", async () => {
            it("배포 계정을 필요한 경우 생성하고, 컨트랙트를 배포함", async () => {
                try {
                    contractTester = await bc.createAccountByCreator(
                        manager,
                        "led"
                    );
                    await contractTester.setContract("lemonade.c");
                    expect(contractTester.accountName).eq(manager);
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: account is already created: ${error}`
                    );
                    errorCount += 1;
                    contractTester = await bc.getAccount(manager);
                    expect(contractTester.accountName).eq(manager);
                }
            });
            it("테스트를 위한 추가 계정들을 생성함", async () => {
                try {
                    // ledtokenTester = await bc.getAccount("led.token");
                    // await bc.createAccountByCreator(fee, "led");
                    // await bc.createAccountByCreator(collector, "p");
                    // await bc.createAccountByCreator(creator, "p");
                    // await bc.createAccountByCreator(buyer, "p");
                    // ledTester = await bc.getAccount("led");
                } catch (error) {
                    console.log(
                        `ERROR ${errorCount}: account is already created: ${error}`
                    );
                    errorCount += 1;
                    // ledtokenTester = await bc.getAccount("led.token");
                    // await bc.getAccount(fee);
                    // await bc.getAccount(collector);
                    // await bc.getAccount(creator);
                    // await bc.getAccount(buyer);
                    // ledTester = await bc.getAccount("led");
                }
            });
        });
    }
});
